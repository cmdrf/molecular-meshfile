/*	MeshCompiler.cpp

MIT License

Copyright (c) 2018-2019 Fabian Herb

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "MeshCompiler.h"
#include <molecular/util/MeshUtils.h>

#include <molecular/util/Range.h>
#include <molecular/util/StringUtils.h>

namespace molecular
{
using namespace util;

namespace MeshCompiler
{

void Compile(const std::vector<std::pair<const void*, size_t> >& vertexBuffers,
		const std::vector<std::pair<const void*, size_t> >& indexBuffers,
		const std::vector<std::vector<VertexAttributeInfo>>& vertexDataSets,
		const std::vector<unsigned int>& vertexDataSetVertexCounts,
		const std::vector<IndexBufferInfo>& indexSpecs,
		const float boundsMin[3], const float boundsMax[3],
		WriteStorage& storage
		)
{
	/* Layout:
	  Header + Buffer Specs
	  Index Specs
	  Vertex Data Sets
	  Vertex Specs
	  Buffers */

	MeshFile meshFile;
	meshFile.magic = MeshFile::kMagic;
	meshFile.version = MeshFile::kVersion;
	meshFile.reserved = 0;
	meshFile.numBuffers = vertexBuffers.size() + indexBuffers.size();
	meshFile.numIndexSpecs = indexSpecs.size();
	meshFile.numVertexDataSets = vertexDataSets.size();
	meshFile.indexSpecsOffset = sizeof(MeshFile) + meshFile.numBuffers * sizeof(MeshFile::Buffer);
	meshFile.vertexDataSetsOffset = meshFile.indexSpecsOffset + meshFile.numIndexSpecs * sizeof(IndexBufferInfo);
	for(int i = 0; i < 3; ++i)
	{
		meshFile.boundsMin[i] = boundsMin[i];
		meshFile.boundsMax[i] = boundsMax[i];
	}

	const uint32_t vertexSpecsOffset = meshFile.vertexDataSetsOffset + vertexDataSets.size() * sizeof(MeshFile::VertexDataSet);
	unsigned int totalVertexSpecsCount = 0;
	for(auto& vertexDataSet: vertexDataSets)
		totalVertexSpecsCount += vertexDataSet.size();
	const uint32_t vertexSpecsSize = totalVertexSpecsCount * sizeof(VertexAttributeInfo);

	uint32_t currentOffset = vertexSpecsOffset + vertexSpecsSize;
	const uint32_t headersEnd = currentOffset;
	// Align to 8 bytes
	currentOffset += 8 - (currentOffset & 7);
	const uint32_t buffersStart = currentOffset;

	// Write header:
	storage.Write(&meshFile, sizeof(MeshFile));

	// Write buffer specs:
	for(auto& indexBuffer: indexBuffers)
	{
		MeshFile::Buffer bufferEntry;
		bufferEntry.type = MeshFile::Buffer::Type::kIndex;
		bufferEntry.offset = currentOffset;
		bufferEntry.size = indexBuffer.second;
		bufferEntry.reserved = 0;
		storage.Write(&bufferEntry, sizeof(MeshFile::Buffer));
		currentOffset += bufferEntry.size;

		// Align to 8 bytes
		unsigned int padding = 8 - (currentOffset & 7);
		currentOffset += padding;
	}

	for(auto& vertexBuffer: vertexBuffers)
	{
		MeshFile::Buffer bufferEntry;
		bufferEntry.type = MeshFile::Buffer::Type::kVertex;
		bufferEntry.offset = currentOffset;
		bufferEntry.size = vertexBuffer.second;
		bufferEntry.reserved = 0;
		storage.Write(&bufferEntry, sizeof(MeshFile::Buffer));
		currentOffset += bufferEntry.size;

		// Align to 8 bytes
		unsigned int padding = 8 - (currentOffset & 7);
		currentOffset += padding;
	}

	// Write index specs:
	for(auto idxSpec: indexSpecs)
	{
		storage.Write(&idxSpec, sizeof(IndexBufferInfo));
	}

	// Write vertex data sets
	uint32_t currentVertexSpecOffset = vertexSpecsOffset;
	for(unsigned int i = 0; i < vertexDataSets.size(); ++i)
	{
		auto& vertexSpecs = vertexDataSets[i];

		MeshFile::VertexDataSet dataSet;
		dataSet.numVertexSpecs = vertexSpecs.size();
		dataSet.vertexSpecsOffset = currentVertexSpecOffset;
		dataSet.numVertices = vertexDataSetVertexCounts.at(i);
		dataSet.reserved = 0;
		storage.Write(&dataSet, sizeof(MeshFile::VertexDataSet));

		currentVertexSpecOffset += sizeof(VertexAttributeInfo) * vertexSpecs.size();
	}

	for(auto& vertexDataSet: vertexDataSets)
	{
		auto& vertexSpecs = vertexDataSet;
		for(auto vertexSpec: vertexSpecs)
		{
			// MeshDataSource counts index and vertex buffers individually from 0:
			vertexSpec.buffer += indexBuffers.size();
			// Write:
			storage.Write(&vertexSpec, sizeof(VertexAttributeInfo));
		}
	}

	// Write buffers:
	uint8_t zero[8] = {0};
	storage.Write(zero, buffersStart - headersEnd);
	for(auto& indexBuffer: indexBuffers)
	{
		unsigned int size = indexBuffer.second;
		storage.Write(indexBuffer.first, size);
		unsigned int padding = 8 - (size & 7);
		storage.Write(zero, padding);
	}

	for(auto& vertexBuffer: vertexBuffers)
	{
		unsigned int size = vertexBuffer.second;
		storage.Write(vertexBuffer.first, size);
		unsigned int padding = 8 - (size & 7);
		storage.Write(zero, padding);
	}

}

using QuadRange = Range<std::vector<ObjFile::Quad>::const_iterator>;
using TriangleRange = Range<std::vector<ObjFile::Triangle>::const_iterator>;

/// Convert OBJ mesh data to data for three vertex buffers and one index buffer
/** Index and vertex data is appended to the output vectors. */
void ObjVertexGroupBuffers(
		const ObjFile& objFile,
		const ObjFile::VertexGroup& vg,
		std::vector<uint16_t>& unifiedIndices,
		std::vector<Vector3>& unifiedPositions,
		std::vector<Vector3>& unifiedNormals,
		std::vector<Vector2>& unifiedUvs)
{
	unsigned int endQuad = vg.firstQuad + vg.numQuads;
	unsigned int endTriangle = vg.firstTriangle + vg.numTriangles;
	assert(endQuad <= objFile.GetQuads().size());
	assert(endTriangle <= objFile.GetTriangles().size());
	auto quadsBegin = objFile.GetQuads().begin();
	auto trianglesBegin = objFile.GetTriangles().begin();
	QuadRange quads(quadsBegin + vg.firstQuad, quadsBegin + endQuad);
	TriangleRange triangles(trianglesBegin + vg.firstTriangle, trianglesBegin + endTriangle);

	std::vector<uint16_t> positionIndices, normalIndices, uvIndices;

	{
		std::vector<uint16_t> quadPositionIndices;
		for(auto& quad: quads)
			quadPositionIndices.insert(quadPositionIndices.end(), quad.vertexIndices.begin(), quad.vertexIndices.end());
		positionIndices.resize(quadPositionIndices.size() / 2 * 3);
		MeshUtils::QuadToTriangleIndices(vg.numQuads, quadPositionIndices.data(), positionIndices.data());
	}

	if(vg.hasNormals)
	{
		std::vector<uint16_t> quadNormalIndices;
		for(auto& quad: quads)
			quadNormalIndices.insert(quadNormalIndices.end(), quad.normalIndices.begin(), quad.normalIndices.end());
		normalIndices.resize(quadNormalIndices.size() / 2 * 3);
		MeshUtils::QuadToTriangleIndices(vg.numQuads, quadNormalIndices.data(), normalIndices.data());
	}

	if(vg.hasTexCoords)
	{
		std::vector<uint16_t> quadUvIndices;
		for(auto& quad: quads)
			quadUvIndices.insert(quadUvIndices.end(), quad.texCoordIndices.begin(), quad.texCoordIndices.end());
		uvIndices.resize(quadUvIndices.size() / 2 * 3);
		MeshUtils::QuadToTriangleIndices(vg.numQuads, quadUvIndices.data(), uvIndices.data());
	}

	for(auto& tri: triangles)
	{
		positionIndices.insert(positionIndices.end(), tri.vertexIndices.begin(), tri.vertexIndices.end());
		if(vg.hasNormals)
			normalIndices.insert(normalIndices.end(), tri.normalIndices.begin(), tri.normalIndices.end());
		if(vg.hasTexCoords)
			uvIndices.insert(uvIndices.end(), tri.texCoordIndices.begin(), tri.texCoordIndices.end());
	}

	const uint16_t* normalIndexData = nullptr;
	const Vector3* normalVertexData = nullptr;
	if(vg.hasNormals)
	{
		assert(positionIndices.size() == normalIndices.size());
		normalIndexData = normalIndices.data();
		normalVertexData = objFile.GetNormals().data();
	}

	const uint16_t* uvIndexData = nullptr;
	const Vector2* uvVertexData = nullptr;
	if(vg.hasTexCoords)
	{
		assert(positionIndices.size() == uvIndices.size());
		uvIndexData = uvIndices.data();
		uvVertexData = objFile.GetTexCoords().data();
	}

	MeshUtils::SeparateToUnifiedIndices(
				positionIndices.size(),
				positionIndices.data(),
				normalIndexData,
				uvIndexData,
				objFile.GetVertices().size(), objFile.GetVertices().data(),
				objFile.GetNormals().size(), normalVertexData,
				objFile.GetTexCoords().size(), uvVertexData,
				unifiedIndices,
				unifiedPositions,
				unifiedNormals,
				unifiedUvs);
}

void Compile(ObjFile& objFile, WriteStorage& storage)
{
	auto& vertexGroups = objFile.GetVertexGroups();
	if(vertexGroups.empty())
		throw std::runtime_error("No vertex groups on OBJ file");

	// Check if all vertex group have the same layout:
	bool hasNormals = vertexGroups.front().hasNormals;
	bool hasTexCoords = vertexGroups.front().hasTexCoords;
	for(size_t i = 1; i < vertexGroups.size(); ++i)
	{
		if(vertexGroups[i].hasNormals != hasNormals || vertexGroups[i].hasTexCoords != hasTexCoords)
			throw std::runtime_error("OBJ files with varying normal/UV layout not supported");
	}

	std::vector<uint16_t> indices;
	std::vector<Vector3> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> uvs;
	std::vector<IndexBufferInfo> indexSpecs;

	for(auto& vg: vertexGroups)
	{
		size_t firstIndex = indices.size();

		ObjVertexGroupBuffers(objFile, vg, indices, positions, normals, uvs);

		IndexBufferInfo indexSpec;
		indexSpec.buffer = 0;
		indexSpec.offset = firstIndex * sizeof(uint16_t);
		indexSpec.count = indices.size() - firstIndex;
		indexSpec.mode = IndexBufferInfo::Mode::kTriangles;
		StringUtils::Copy(vg.material, indexSpec.material);
		indexSpec.type = IndexBufferInfo::Type::kUInt16;
		indexSpec.vertexDataSet = 0;
		indexSpecs.push_back(indexSpec);
	}

	std::vector<VertexAttributeInfo> vertexSpecs;
	std::vector<std::pair<const void*, size_t>> vertexBuffers;
	std::vector<uint8_t> interleavedNormalsAndUvs; // Must remain in scope while calling Compile in the end

	vertexBuffers.emplace_back(positions.data(), positions.size() * sizeof(Vector3));
	VertexAttributeInfo positionSpec;
	positionSpec.buffer = 0;
	positionSpec.components = 3;
	positionSpec.normalized = true;
	positionSpec.offset = 0;
	positionSpec.semantic = VertexAttributeInfo::kPosition;
	positionSpec.stride = 0;
	positionSpec.type = VertexAttributeInfo::Type::kFloat;
	vertexSpecs.push_back(positionSpec);

	if(!normals.empty() && !uvs.empty())
	{
		assert(normals.size() == uvs.size());
		size_t count = normals.size();
		size_t interleavedDatumSize = sizeof(Vector3) + sizeof(Vector2);
		interleavedNormalsAndUvs.resize(count * interleavedDatumSize);
		MeshUtils::Interleave(count, sizeof(Vector3), sizeof(Vector2), normals.data(), uvs.data(), interleavedNormalsAndUvs.data());
		vertexBuffers.emplace_back(interleavedNormalsAndUvs.data(), interleavedNormalsAndUvs.size());

		VertexAttributeInfo normalsSpec;
		normalsSpec.buffer = 1;
		normalsSpec.components = 3;
		normalsSpec.normalized = true;
		normalsSpec.offset = 0;
		normalsSpec.semantic = VertexAttributeInfo::kNormal;
		normalsSpec.stride = interleavedDatumSize;
		normalsSpec.type = VertexAttributeInfo::Type::kFloat;
		vertexSpecs.push_back(normalsSpec);

		VertexAttributeInfo uvSpec;
		uvSpec.buffer = 1;
		uvSpec.components = 2;
		uvSpec.normalized = true;
		uvSpec.offset = sizeof(Vector3);
		uvSpec.semantic = VertexAttributeInfo::kTextureCoords;
		uvSpec.stride = interleavedDatumSize;
		uvSpec.type = VertexAttributeInfo::Type::kFloat;
		vertexSpecs.push_back(uvSpec);
	}
	else if(!normals.empty())
	{
		vertexBuffers.emplace_back(normals.data(), normals.size() * sizeof(Vector3));
		VertexAttributeInfo normalsSpec;
		normalsSpec.buffer = 1;
		normalsSpec.components = 3;
		normalsSpec.normalized = true;
		normalsSpec.offset = 0;
		normalsSpec.semantic = VertexAttributeInfo::kNormal;
		normalsSpec.stride = 0;
		normalsSpec.type = VertexAttributeInfo::Type::kFloat;
		vertexSpecs.push_back(normalsSpec);
	}
	else if(!uvs.empty())
	{
		vertexBuffers.emplace_back(uvs.data(), uvs.size() * sizeof(Vector2));
		VertexAttributeInfo uvSpec;
		uvSpec.buffer = 1;
		uvSpec.components = 2;
		uvSpec.normalized = true;
		uvSpec.offset = 0;
		uvSpec.semantic = VertexAttributeInfo::kTextureCoords;
		uvSpec.stride = 0;
		uvSpec.type = VertexAttributeInfo::Type::kFloat;
		vertexSpecs.push_back(uvSpec);
	}

	auto bounds = objFile.GetBoundingBox();

	std::vector<std::pair<const void*, size_t>> indexBuffers;
	indexBuffers.emplace_back(indices.data(), indices.size() * sizeof(uint16_t));
	Compile(vertexBuffers, indexBuffers,
			std::vector<std::vector<VertexAttributeInfo>>(1, vertexSpecs),
			std::vector<unsigned int>(1, positions.size()),
			indexSpecs,
			bounds.GetMin(), bounds.GetMax(),
			storage);
}

MeshSet ObjFileToMeshSet(ObjFile& objFile)
{
	auto& vertexGroups = objFile.GetVertexGroups();
	MeshSet meshSet;

	for(auto& vg: vertexGroups)
	{
		if(vg.numQuads == 0 && vg.numTriangles == 0)
			continue;

		std::vector<uint16_t> indices;
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> uvs;

		ObjVertexGroupBuffers(objFile, vg, indices, positions, normals, uvs);
		size_t numVertices = positions.size();
		assert(normals.empty() || normals.size() == numVertices);
		assert(uvs.empty() || uvs.size() == numVertices);

		meshSet.emplace_back(numVertices);
		Mesh& mesh = meshSet.back();
		mesh.SetAttributeData(VertexAttributeInfo::kPosition, positions.data(), positions.size());
		if(!normals.empty())
			mesh.SetAttributeData(VertexAttributeInfo::kNormal, normals.data(), normals.size());
		if(!uvs.empty())
			mesh.SetAttributeData(VertexAttributeInfo::kTextureCoords, uvs.data(), uvs.size());

		mesh.SetMaterial(vg.material);

		auto& outIndices = mesh.GetIndices();
		outIndices.reserve(indices.size());
		for(uint16_t index: indices)
			outIndices.push_back(index);
	}

	return meshSet;
}

void Compile(const MeshSet& meshes, WriteStorage& storage)
{
	std::vector<std::pair<const void*, size_t>> indexBuffers;
	std::vector<std::pair<const void*, size_t>> vertexBuffers;
	std::vector<std::vector<VertexAttributeInfo>> vertexDataSets;
	std::vector<unsigned int> vertexDataSetVertexCounts;
	std::vector<IndexBufferInfo> indexSpecs;
	util::AxisAlignedBox bounds;

	for(auto& mesh: meshes)
	{
		auto& indices = mesh.GetIndices();
		IndexBufferInfo indexSpec;
		indexSpec.buffer = indexBuffers.size();
		indexSpec.count = indices.size();
		StringUtils::Copy(mesh.GetMaterial(), indexSpec.material);
		indexSpec.mode = mesh.GetMode();
		indexSpec.offset = 0;
		indexSpec.type = IndexBufferInfo::Type::kUInt32;
		indexSpec.vertexDataSet = vertexDataSets.size();
		indexBuffers.emplace_back(indices.data(), indices.size() * sizeof(uint32_t)); // TODO: Use smaller index types when possible
		indexSpecs.push_back(indexSpec);

		std::vector<VertexAttributeInfo> vertexSpecs;
		auto& attributes = mesh.GetAttributes();
		for(auto& attribute: attributes)
		{
			VertexAttributeInfo vertexSpec;
			vertexSpec.buffer = vertexBuffers.size();
			vertexSpec.components = attribute.second.GetNumComponents();
			vertexSpec.normalized = true;
			vertexSpec.offset = 0;
			vertexSpec.semantic = attribute.first;
			vertexSpec.stride = 0;
			vertexSpec.type = attribute.second.GetType();
			vertexSpecs.push_back(vertexSpec);
			vertexBuffers.emplace_back(attribute.second.GetRawData(), attribute.second.GetRawSize());

			if(attribute.first == VertexAttributeInfo::kPosition)
			{
				const Vector3* positions = attribute.second.GetData<Vector3>();
				for(size_t i = 0; i < mesh.GetNumVertices(); ++i)
					bounds.Stretch(positions[i]);
			}
		}
		vertexDataSets.push_back(vertexSpecs);
		vertexDataSetVertexCounts.push_back(mesh.GetNumVertices());
	}


	Compile(vertexBuffers, indexBuffers,
			vertexDataSets,
			vertexDataSetVertexCounts,
			indexSpecs,
			bounds.GetMin(), bounds.GetMax(),
			storage);
}

} // namespace

} // namespace molecular
