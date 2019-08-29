/*	MeshUtils.cpp

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

#include "MeshUtils.h"

#include <molecular/util/FloatToHalf.h>

#include <cassert>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

namespace molecular
{

namespace MeshUtils
{

void Interleave(size_t count, size_t datumSize0, size_t datumSize1, const void* data0, const void* data1, void* outData)
{
	const uint8_t* bytes0 = static_cast<const uint8_t*>(data0);
	const uint8_t* bytes1 = static_cast<const uint8_t*>(data1);
	uint8_t* outBytes = static_cast<uint8_t*>(outData);

	size_t stride = datumSize0 + datumSize1;
	for(size_t i = 0; i < count; ++i)
	{
		std::memcpy(outBytes + stride * i, bytes0 + datumSize0 * i, datumSize0);
		std::memcpy(outBytes + stride * i + datumSize0, bytes1 + datumSize1 * i, datumSize1);
	}
}

template<typename T>
void QuadToTriangleIndices(size_t quadCount, const T in[], T out[])
{
	for(size_t i = 0; i < quadCount; ++i)
	{
		assert(in);
		assert(out);
		out[i * 6 + 0] = in[i * 4 + 0];
		out[i * 6 + 1] = in[i * 4 + 1];
		out[i * 6 + 2] = in[i * 4 + 2];
		out[i * 6 + 3] = in[i * 4 + 0];
		out[i * 6 + 4] = in[i * 4 + 2];
		out[i * 6 + 5] = in[i * 4 + 3];
	}
}

template void QuadToTriangleIndices<uint8_t>(size_t quadCount, const uint8_t in[], uint8_t out[]);
template void QuadToTriangleIndices<uint16_t>(size_t quadCount, const uint16_t in[], uint16_t out[]);
template void QuadToTriangleIndices<uint32_t>(size_t quadCount, const uint32_t in[], uint32_t out[]);

void Scale(Mesh& mesh, float scaleFactor)
{
	Vector3* positions = mesh.GetAttribute(VertexAttributeInfo::kPosition).GetData<Vector3>();
	unsigned int numVertices = mesh.GetNumVertices();
	for(unsigned int i = 0; i < numVertices; ++i)
		positions[i] *= scaleFactor;
}

Vector3 TriangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3)
{
	Vector3 u = p2 - p1;
	Vector3 v = p3 - p1;
	return u.CrossProduct(v).Normalized();
}

std::vector<Vector3> IndexedTriangleNormals(const std::vector<Vector3>& positions, const int triangleIndices[], size_t triangleCount)
{
	std::vector<Vector3> normals(positions.size(), Vector3(0, 0, 0));
	for(size_t i = 0; i < triangleCount; ++i)
	{
		unsigned int idx1 = triangleIndices[i * 3];
		unsigned int idx2 = triangleIndices[i * 3 + 1];
		unsigned int idx3 = triangleIndices[i * 3 + 2];
		Vector3 p1 = positions.at(idx1);
		Vector3 p2 = positions.at(idx2);
		Vector3 p3 = positions.at(idx3);
		Vector3 normal = TriangleNormal(p1, p2, p3);
		normals[idx1] += normal;
		normals[idx2] += normal;
		normals[idx3] += normal;
	}
	for(auto& normal: normals)
		normal = normal.Normalized();

	return normals;
}

std::vector<int> TriangleNeighbours(const int triangleIndices[], unsigned int triangleCount)
{
	std::vector<int> out(triangleCount * 3, -1);
	std::unordered_map<int64_t, unsigned int> edgeNeighbours;
	for(unsigned int i = 0; i < triangleCount; ++i)
	{
		const int64_t idx0 = triangleIndices[i * 3];
		const int64_t idx1 = triangleIndices[i * 3 + 1];
		const int64_t idx2 = triangleIndices[i * 3 + 2];
		const int64_t combinedIdx0 = (idx1 << 32) | idx2; // Edge opposite of vertex 0
		const int64_t combinedIdx1 = (idx2 << 32) | idx0; // Edge opposite of vertex 1
		const int64_t combinedIdx2 = (idx0 << 32) | idx1; // Edge opposite of vertex 2
		assert(edgeNeighbours.find(combinedIdx0) == edgeNeighbours.end() && "Two or more triangles share an edge in the same orientation");
		assert(edgeNeighbours.find(combinedIdx1) == edgeNeighbours.end() && "Two or more triangles share an edge in the same orientation");
		assert(edgeNeighbours.find(combinedIdx2) == edgeNeighbours.end() && "Two or more triangles share an edge in the same orientation");
		edgeNeighbours[combinedIdx0] = i;
		edgeNeighbours[combinedIdx1] = i;
		edgeNeighbours[combinedIdx2] = i;
	}

	// Query edges in opposite direction:
	for(unsigned int i = 0; i < triangleCount; ++i)
	{
		int64_t idx0 = triangleIndices[i * 3];
		int64_t idx1 = triangleIndices[i * 3 + 1];
		int64_t idx2 = triangleIndices[i * 3 + 2];
		out[i * 3] = edgeNeighbours.at((idx2 << 32) | idx1); // Edge opposite of vertex 0
		out[i * 3 + 1] = edgeNeighbours.at((idx0 << 32) | idx2); // Edge opposite of vertex 1
		out[i * 3 + 2] = edgeNeighbours.at((idx1 << 32) | idx0); // Edge opposite of vertex 2
	}
	return out;
}

void Transform(Mesh& mesh, const Matrix4& transform)
{
	for(auto& attribute: mesh.GetAttributes())
	{
		if(attribute.first == VertexAttributeInfo::kPosition)
		{
			Vector3* positionData = attribute.second.GetData<Vector3>();
			for(unsigned int i = 0; i < mesh.GetNumVertices(); ++i)
			{
				Vector4 p = transform * Vector4(positionData[i], 1.0f);
				positionData[i] = Vector3(p[0] / p[3], p[1] / p[3], p[2] / p[3]);
			}
		}
		else if(attribute.first == VertexAttributeInfo::kNormal)
		{
			Vector3* normalData = attribute.second.GetData<Vector3>();
			for(unsigned int i = 0; i < mesh.GetNumVertices(); ++i)
			{
				Vector4 p = transform * Vector4(normalData[i], 0.0f);
				normalData[i] = Vector3(p[0] / p[3], p[1] / p[3], p[2] / p[3]);
			}
		}
	}
}

void ReducePrecision(Mesh& mesh)
{
	FloatToHalf fth;
	std::unordered_set<Hash> toHalf = {
		VertexAttributeInfo::kVertexPrt0,
		VertexAttributeInfo::kVertexPrt1,
		VertexAttributeInfo::kVertexPrt2,
		VertexAttributeInfo::kNormal,
		VertexAttributeInfo::kSkinWeights
	};
	std::unordered_set<Hash> toInt8 = {
		VertexAttributeInfo::kSkinJoints
	};

	auto& attributes = mesh.GetAttributes();
	for(auto& attribute: attributes)
	{
		if(toHalf.count(attribute.first) && attribute.second.GetType() == VertexAttributeInfo::kFloat)
		{
			const float* floatData = static_cast<const float*>(attribute.second.GetRawData());
			size_t floatCount = attribute.second.GetRawSize() / 4;
			std::vector<uint16_t> halfData(floatCount);
			for(size_t i = 0; i < floatCount; i++)
				halfData[i] = fth.Convert(floatData[i]);
			attribute.second.SetData(VertexAttributeInfo::kHalf, attribute.second.GetNumComponents(), halfData.data(), floatCount * 2);
		}
		else if(toInt8.count(attribute.first) && attribute.second.GetType() == VertexAttributeInfo::kInt32)
		{
			const int32_t* intData = static_cast<const int32_t*>(attribute.second.GetRawData());
			size_t intCount = attribute.second.GetRawSize() / 4;
			std::vector<int8_t> int8Data(intCount);
			for(size_t i = 0; i < intCount; i++)
				int8Data[i] = int8_t(intData[i]);
			attribute.second.SetData(VertexAttributeInfo::kInt8, attribute.second.GetNumComponents(), int8Data.data(), intCount);
		}
	}
}

} // namespace MeshUtils

} // namespace molecular
