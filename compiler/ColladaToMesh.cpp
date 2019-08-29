/*	ColladaToMesh.cpp

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

#include "CharacterAnimation.h"
#include "ColladaToMesh.h"
#include "Mesh.h"
#include "MeshUtils.h"

#include <molecular/util/StringUtils.h>

#include <algorithm>
#include <array>

namespace molecular
{

namespace ColladaToMesh
{

void ReadMeshData(
		const ColladaFile::Mesh& mesh,
		std::vector<uint16_t>& outPositionIndices,
		std::vector<uint16_t>& outNormalIndices,
		std::vector<uint16_t>& outTexCoordIndices,
		std::vector<Vector3>& outPositions,
		std::vector<Vector3>& outNormals,
		std::vector<Vector2>& outTexCoords)
{
	std::vector<ColladaFile::Input> inputs;
	std::vector<int> primitives;
	if(mesh.HasPolylist())
	{
		auto polylist = mesh.GetPolylist();
		inputs = polylist.GetInputs();
		primitives = polylist.GetPrimitives();
		for(auto vertexCount: polylist.GetVertexCounts())
			if(vertexCount != 3)
				throw std::runtime_error("Only polygons with 3 vertices supported");
	}
	else if(mesh.HasTriangles())
	{
		auto triangles = mesh.GetTriangles();
		inputs = triangles.GetInputs();
		primitives = triangles.GetPrimitives();
	}
	else
		throw std::runtime_error("Neither polylist nor triangles in mesh");

	std::vector<float> positions, normals, texcoord;
	int vertexOffset = -1;
	int normalOffset = -1;
	int texCoordOffset = -1;
	for(auto& input: inputs)
	{
		const char* semantic = input.GetSemantic();
		const char* sourceUrl = input.GetSource();
		if(StringUtils::Equals(semantic, "VERTEX"))
		{
			auto vertices = mesh.GetVertices(sourceUrl + 1); // Skip '#'
			vertexOffset = input.GetOffset();
			for(auto& verticesInput: vertices.GetInputs())
			{
				if(StringUtils::Equals(verticesInput.GetSemantic(), "POSITION"))
				{
					auto positionSource = mesh.GetSource(verticesInput.GetSource() + 1);
					auto accessor = positionSource.GetTechniqueCommon().GetAccessor();
					positions = positionSource.GetFloatArray(accessor.GetSource() + 1);

				}
			}
		}
		else
		{
			auto source = mesh.GetSource(sourceUrl + 1);
			auto accessor = source.GetTechniqueCommon().GetAccessor();
			if(StringUtils::Equals(semantic, "NORMAL"))
			{
				normals = source.GetFloatArray(accessor.GetSource() + 1);
				normalOffset = input.GetOffset();
			}
			else if(StringUtils::Equals(semantic, "TEXCOORD"))
			{
				texcoord = source.GetFloatArray(accessor.GetSource() + 1);
				texCoordOffset = input.GetOffset();
			}
			else // Cannot skip because primitive offsets would be wrong
				throw std::runtime_error(std::string("Unsupported semantic \"") + semantic + "\"");
		}
	}

	int primitiveStride = std::max(vertexOffset, std::max(normalOffset, texCoordOffset)) + 1;
	if(primitiveStride < 1)
		throw std::runtime_error("Invalid primitive stride");
	size_t primitiveCount = primitives.size() / primitiveStride;
	outPositionIndices.resize(vertexOffset >= 0 ? primitiveCount : 0);
	outNormalIndices.resize(normalOffset >= 0 ? primitiveCount : 0);
	outTexCoordIndices.resize(texCoordOffset >= 0 ? primitiveCount : 0);
	for(unsigned int i = 0; i < primitiveCount; ++i)
	{
		if(vertexOffset >= 0)
			outPositionIndices[i] = primitives[i * primitiveStride + vertexOffset];
		if(normalOffset >= 0)
			outNormalIndices[i] = primitives[i * primitiveStride + normalOffset];
		if(texCoordOffset >= 0)
			outTexCoordIndices[i] = primitives[i * primitiveStride + texCoordOffset];
	}

	size_t numPositions = positions.size() / 3;
	size_t numNormals = normals.size() / 3;
	size_t numTexCoords = texcoord.size() / 2;
	outPositions.resize(numPositions);
	outNormals.resize(numNormals);
	outTexCoords.resize(numTexCoords);
	for(size_t i = 0; i < numPositions; i++)
		outPositions[i] = Vector3(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
	for(size_t i = 0; i < numNormals; i++)
		outNormals[i] = Vector3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
	for(size_t i = 0; i < numTexCoords; i++)
		outTexCoords[i] = Vector2(texcoord[i * 2], texcoord[i * 2 + 1]);

	// Swap V:
	for(auto& texCoord: outTexCoords)
		texCoord = Vector2(texCoord[0], 1.0f - texCoord[1]);

}

Mesh ToMesh(
		const std::vector<uint16_t>& indices,
		const std::vector<Vector3>& positions,
		const std::vector<Vector3>& normals,
		const std::vector<Vector2>& texCoords)
{
	assert(normals.empty() || normals.size() == positions.size());
	assert(texCoords.empty() || texCoords.size() == positions.size());
	Mesh outMesh(positions.size());
	outMesh.SetAttributeData("vertexPositionAttr"_H, positions.data(), positions.size());
	if(!normals.empty())
		outMesh.SetAttributeData("vertexNormalAttr"_H, normals.data(), normals.size());
	if(!texCoords.empty())
		outMesh.SetAttributeData(VertexAttributeInfo::kTextureCoords, texCoords.data(), texCoords.size());
	outMesh.SetMode(IndexBufferInfo::Mode::kTriangles);
	auto& outIndices32 = outMesh.GetIndices();
	outIndices32.reserve(indices.size());
	for(uint16_t index: indices)
		outIndices32.push_back(index);
	return outMesh;
}

const char* GetMaterial(const ColladaFile& file, const ColladaFile::Mesh& mesh)
{
	const char* material = nullptr;
	if(mesh.HasPolylist())
		material = mesh.GetPolylist().GetMaterial();
	else if(mesh.HasTriangles())
		material = mesh.GetTriangles().GetMaterial();
	if(material)
		return file.GetMaterial(material).GetName();
	else
		return "";
}

Mesh ToMesh(const ColladaFile& file, const ColladaFile::Mesh& mesh)
{
	std::vector<uint16_t> positionIndices;
	std::vector<uint16_t> normalIndices;
	std::vector<uint16_t> texCoordIndices;
	std::vector<Vector3> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texCoords;
	ReadMeshData(mesh, positionIndices, normalIndices, texCoordIndices, positions, normals, texCoords);

	std::vector<uint16_t> outIndices;
	std::vector<Vector3> outPositions;
	std::vector<Vector3> outNormals;
	std::vector<Vector2> outUvs;
	MeshUtils::SeparateToUnifiedIndices(
				positionIndices.size(),
				positionIndices.data(),
				normalIndices.empty() ? nullptr : normalIndices.data(),
				texCoordIndices.empty() ? nullptr : texCoordIndices.data(),
				positions.size(),
				positions.data(),
				normals.size(),
				normals.data(),
				texCoords.size(),
				texCoords.data(),
				outIndices,
				outPositions,
				outNormals,
				outUvs);
	Mesh outMesh = ToMesh(outIndices, outPositions, outNormals, outUvs);
	outMesh.SetMaterial(GetMaterial(file, mesh));
	return outMesh;
}

Mesh ToMesh(const ColladaFile& file, const ColladaFile::Skin& skin)
{
	auto mesh = file.GetGeometry(skin.GetSource() + 1).GetMesh();
	std::vector<uint16_t> positionIndices;
	std::vector<uint16_t> normalIndices;
	std::vector<uint16_t> texCoordIndices;
	std::vector<Vector3> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texCoords;
	ReadMeshData(mesh, positionIndices, normalIndices, texCoordIndices, positions, normals, texCoords);

	std::vector<Vector4> vertexWeights;
	std::vector<IntVector4> vertexJoints;
	ReadVertexWeights(skin, vertexWeights, vertexJoints);

	if(vertexWeights.size() != positions.size() || vertexJoints.size() != positions.size())
		throw std::runtime_error("Number of positions and vertex weights not matching");

	struct Vertex
	{
		Vector3 position;
		Vector4 weights;
		IntVector4 joints;
	};

	// Struct of vectors to Vector of structs:
	std::vector<Vertex> vertices(positions.size());
	for(size_t i = 0; i < positions.size(); i++)
	{
		vertices[i].position = positions[i];
		vertices[i].weights = vertexWeights[i];
		vertices[i].joints = vertexJoints[i];
	}

	std::vector<uint16_t> outIndices;
	std::vector<Vertex> outVertices;
	std::vector<Vector3> outNormals;
	std::vector<Vector2> outUvs;
	MeshUtils::SeparateToUnifiedIndices(
				positionIndices.size(),
				positionIndices.data(),
				normalIndices.empty() ? nullptr : normalIndices.data(),
				texCoordIndices.empty() ? nullptr : texCoordIndices.data(),
				vertices.size(),
				vertices.data(),
				normals.size(),
				normals.data(),
				texCoords.size(),
				texCoords.data(),
				outIndices,
				outVertices,
				outNormals,
				outUvs);

	// Vector of structs to struct of vectors:
	std::vector<Vector3> outPositions(outVertices.size());
	std::vector<Vector4> outVertexWeights(outVertices.size());
	std::vector<IntVector4> outVertexJoints(outVertices.size());
	for(size_t i = 0; i < outVertices.size(); i++)
	{
		outPositions[i] = outVertices[i].position;
		outVertexWeights[i] = outVertices[i].weights;
		outVertexJoints[i] = outVertices[i].joints;
	}

	Mesh outMesh = ToMesh(outIndices, outPositions, outNormals, outUvs);
	outMesh.SetAttributeData("vertexSkinWeightsAttr"_H, outVertexWeights.data(), outVertexWeights.size());
	outMesh.SetAttributeData("vertexSkinJointsAttr"_H, outVertexJoints.data(), outVertexJoints.size());
	outMesh.SetMaterial(GetMaterial(file, mesh));
	return outMesh;
}

MeshSet ToMesh(const ColladaFile& file, const ColladaFile::Node& node)
{
	MeshSet out;
	if(node.HasInstanceGeometry())
	{
		const char* url = node.GetInstanceGeometry().GetUrl();
		out.push_back(ToMesh(file, file.GetGeometry(url + 1).GetMesh()));
	}
	if(node.HasInstanceController())
	{
		const char* url = node.GetInstanceController().GetUrl();
		auto controller = file.GetController(url + 1);
		auto skin = controller.GetSkin();
		out.push_back(ToMesh(file, skin));
	}
	for(auto& n: node.GetNodes())
	{
		// Recurse into child nodes
		auto meshes = ToMesh(file, n);
		for(auto& mesh: meshes)
			out.push_back(std::move(mesh));
//		out.insert(out.end(), std::make_move_iterator(meshes.begin()), std::make_move_iterator(meshes.end()));
	}
	Matrix4 matrix = node.GetMatrix();
	for(auto& mesh: out)
		MeshUtils::Transform(mesh, matrix);
	return out;
}

MeshSet ToMesh(const ColladaFile& file, const ColladaFile::VisualScene& scene)
{
	MeshSet out;
	for(auto& node: scene.GetNodes())
	{
		auto meshes = ToMesh(file, node);
		for(auto& mesh: meshes)
			out.push_back(std::move(mesh));
//		out.insert(out.end(), std::make_move_iterator(meshes.begin()), std::make_move_iterator(meshes.end()));
	}
	return out;
}

MeshSet ToMesh(const ColladaFile& file)
{
	const char* sceneUrl = file.GetScene().GetInstanceVisualSceneUrl();
	return ToMesh(file, file.GetVisualScene(sceneUrl + 1));
}

void ReadInverseBindMatrices(
		const ColladaFile::Skin& skin,
		std::vector<Hash>& jointNames,
		std::vector<Matrix4>& inverseBindMatrices)
{
	const ColladaFile::Joints joints = skin.GetJoints();
	for(auto& input: joints.GetInputs())
	{
		const char* semantic = input.GetSemantic();
		auto source = skin.GetSource(input.GetSource() + 1);
		auto accessor = source.GetTechniqueCommon().GetAccessor();
		if(StringUtils::Equals(semantic, "JOINT"))
		{
			jointNames = source.GetNameArray(accessor.GetSource() + 1);
		}
		else if(StringUtils::Equals(semantic, "INV_BIND_MATRIX"))
		{
			inverseBindMatrices = source.GetFloatArrayAsMatrices(accessor.GetSource() + 1);
		}
	}
}

/// Convert joint indices used in the collada file to indices used by the engine
void FileToEngineJointIndices(const std::vector<Hash> fileJointNames, std::vector<IntVector4>& jointIndices)
{
	for(auto& indices: jointIndices)
	{
		for(int i = 0; i < 4; i++)
		{
			if(indices[i] >= 0)
			{
				auto index = indices[i];
				if(index >= fileJointNames.size())
					throw std::runtime_error("Joint index out of bounds");
				Hash name = fileJointNames[index];
				indices[i] = CharacterAnimation::GetBoneIndex(name);
			}
		}
	}
}

void ReadVertexWeights(
		const ColladaFile::Skin& skin,
		std::vector<Vector4>& outWeights,
		std::vector<IntVector4>& outJointIndices)
{
	auto vertexWeights = skin.GetVertexWeights();
	int count = vertexWeights.GetCount();
	outWeights.reserve(count);
	outJointIndices.reserve(count);
	auto vertexCounts = vertexWeights.GetVCount();
	auto vertices = vertexWeights.GetV();
	std::vector<Hash> jointNames;
	std::vector<float> weights;
	int jointOffset = -1;
	int weightOffset = -1;
	for(auto& input: vertexWeights.GetInputs())
	{
		const char* semantic = input.GetSemantic();
		auto source = skin.GetSource(input.GetSource() + 1);
		auto accessor = source.GetTechniqueCommon().GetAccessor();
		if(StringUtils::Equals(semantic, "JOINT"))
		{
			jointNames = source.GetNameArray(accessor.GetSource() + 1);
			jointOffset = input.GetOffset();
		}
		else if(StringUtils::Equals(semantic, "WEIGHT"))
		{
			weights = source.GetFloatArray(accessor.GetSource() + 1);
			weightOffset = input.GetOffset();
		}
		else
			throw std::runtime_error(std::string("Unkown input \"") + semantic + "\"");
	}
	if(weights.empty())
		throw std::runtime_error("No weights array found.");
	if(jointOffset != 0 || weightOffset != 1)
		throw std::runtime_error("Only jointOffset = 0, weightOffset = 1 supported. Implement proper offset handling!");

	auto vertexIt = vertices.begin();
	for(int vertexCount: vertexCounts)
	{
		Vector4 weightsVector(0, 0, 0, 0);
		IntVector4 indicesVector(-1, -1, -1, -1);
		if(vertexCount <= 4)
		{
			for(int i = 0; i < vertexCount; i++)
			{
				indicesVector[i] = *vertexIt;
				vertexIt++;
				weightsVector[i] = weights.at(*vertexIt);
				vertexIt++;
			}
		}
		else if(vertexCount <= 10)
		{
			struct VertexInfluence
			{
				int joint = -1;
				float weight = 0;
			};
			std::array<VertexInfluence, 10> vertexInfluences;
			for(unsigned int i = 0; i < vertexCount; i++)
			{
				vertexInfluences[i].joint = *vertexIt;
				vertexIt++;
				vertexInfluences[i].weight = weights.at(*vertexIt);
				vertexIt++;
			}

			// Sort, so that the first four elements are the largest:
			std::partial_sort(
						vertexInfluences.begin(),
						vertexInfluences.begin() + 4,
						vertexInfluences.begin() + vertexCount,
						[](auto& vi0, auto& vi1){return vi0.weight > vi1.weight;});

			for(unsigned int i = 0; i < 4; i++)
			{
				indicesVector[i] = vertexInfluences[i].joint;
				weightsVector[i] = vertexInfluences[i].weight;
			}
		}
		else
			throw std::runtime_error("Too many influencing joints for one vertex");

		outJointIndices.push_back(indicesVector);
		outWeights.push_back(weightsVector.Normalized()); // Normalize
	}
	FileToEngineJointIndices(jointNames, outJointIndices);
}

}

} // namespace molecular
