/*	MeshCompiler.cpp

MIT License

Copyright (c) 2018-2023 Fabian Herb

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
#include <molecular/util/ObjFileUtils.h>
#include <molecular/util/Range.h>
#include <molecular/util/StringUtils.h>

namespace molecular
{
using namespace util;
using namespace meshfile;

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

MeshSet ObjFileToMeshSet(ObjFile& objFile)
{
	auto& vertexGroups = objFile.GetVertexGroups();
	MeshSet meshSet;

	for(auto& vg: vertexGroups)
	{
		if(vg.numQuads == 0 && vg.numTriangles == 0)
			continue;

		std::vector<uint32_t> indices;
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> uvs;

		ObjFileUtils::ObjVertexGroupBuffers(objFile, vg, indices, positions, normals, uvs);
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
		for(uint32_t index: indices)
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
