/*	MeshUtils.h

MIT License

Copyright (c) 2018 Fabian Herb

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

#ifndef MESHUTILS_H
#define MESHUTILS_H

#include "util/Vector3.h"
#include "util/Matrix4.h"
#include "Mesh.h"
#include <vector>

namespace MeshUtils
{
/// Convert seperate indices as found in OBJ files to unified ones
/** In OBJ and COLLADA files, each face has individual indices to the vertex, normal
	and UV buffers. OpenGL only allows for the same index to each buffer,
	so this method generates new indices for each combination of vertex and
	UV indices by duplicating the missing vertex information as required.

	Index and vertex data is appended to the output vectors.

	@param indices0 Array with numIndices elements for attribute 0. May be nullptr.
	@param indices1 Array with numIndices elements for attribute 1. May be nullptr. */
template<class Attribute0, class Attribute1>
void SeparateToUnifiedIndices(
		size_t numIndices,
		const uint16_t indices0[],
		const uint16_t indices1[],
		size_t numAttributes0, const Attribute0 attributes0[],
		size_t numAttributes1, const Attribute1 attributes1[],
		std::vector<uint16_t>& outIndices,
		std::vector<Attribute0>& outAttributes0,
		std::vector<Attribute1>& outAttributes1);

template<class Attribute0, class Attribute1, class Attribute2>
void SeparateToUnifiedIndices(
		size_t numIndices,
		const uint16_t indices0[],
		const uint16_t indices1[],
		const uint16_t indices2[],
		size_t numAttributes0, const Attribute0 attributes0[],
		size_t numAttributes1, const Attribute1 attributes1[],
		size_t numAttributes2, const Attribute2 attributes2[],
		std::vector<uint16_t>& outIndices,
		std::vector<Attribute0>& outAttributes0,
		std::vector<Attribute1>& outAttributes1,
		std::vector<Attribute2>& outAttributes2);

/** @param count Count of datums in data0 and data1. Size of data0 must be count times datumSize0 and size of data1 must be count times datumSize1.
	@param outData Pointer to buffer that has the size of data0 and data1 combined. */
void Interleave(size_t count, size_t datumSize0, size_t datumSize1, const void* data0, const void* data1, void* outData);

/// Convert quad indices to triangle indices
/** @param out Must have 3/2 times the size of in. */
template<typename T>
void QuadToTriangleIndices(size_t quadCount, const T in[], T out[]);

void Scale(Mesh& mesh, float scaleFactor);

Vector3 TriangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3);

std::vector<Vector3> IndexedTriangleNormals(const std::vector<Vector3>& positions, const int triangleIndices[], size_t triangleCount);

std::vector<int> TriangleNeighbours(const int triangleIndices[], unsigned int triangleCount);

/** Handles only position and normal attributes.
	@todo Handle more attribute types properly. */
void Transform(Mesh& mesh, const Matrix4& transform);

/// Use half floats or integer types where appropriate
void ReducePrecision(Mesh& mesh);

/*****************************************************************************/

template<class Attribute0, class Attribute1>
void SeparateToUnifiedIndices(
		size_t numIndices,
		const uint16_t indices0[],
		const uint16_t indices1[],
		size_t numAttributes0, const Attribute0 attributes0[],
		size_t numAttributes1, const Attribute1 attributes1[],
		std::vector<uint16_t>& outIndices,
		std::vector<Attribute0>& outAttributes0,
		std::vector<Attribute1>& outAttributes1)
{
	if(indices0 && indices1)
	{
		std::unordered_map<uint32_t, uint16_t> vertexMap;

		for(size_t i = 0; i < numIndices; ++i)
		{
			uint16_t index0 = indices0[i];
			uint16_t index1 = indices1[i];

			uint32_t combinedIndex = (uint32_t(index0) << 16) | index1;
			auto it = vertexMap.find(combinedIndex);

			if(it == vertexMap.end())
			{
				uint16_t outIndex = outAttributes0.size();
				vertexMap[combinedIndex] = outIndex;
				outIndices.push_back(outIndex);

				assert(numAttributes0 > index0);
				outAttributes0.push_back(attributes0[index0]);

				assert(numAttributes1 > index1);
				outAttributes1.push_back(attributes1[index1]);
			}
			else
				outIndices.push_back(it->second);
		}
	}
	else if(indices0)
	{
		outIndices.insert(outIndices.end(), indices0, indices0 + numIndices);
		outAttributes0.insert(outAttributes0.end(), attributes0, attributes0 + numAttributes0);
	}
	else if(indices1)
	{
		outIndices.insert(outIndices.end(), indices1, indices1 + numIndices);
		outAttributes1.insert(outAttributes1.end(), attributes1, attributes1 + numAttributes1);
	}
}

template<class Attribute0, class Attribute1, class Attribute2>
void SeparateToUnifiedIndices(
		size_t numIndices,
		const uint16_t indices0[],
		const uint16_t indices1[],
		const uint16_t indices2[],
		size_t numAttributes0, const Attribute0 attributes0[],
		size_t numAttributes1, const Attribute1 attributes1[],
		size_t numAttributes2, const Attribute2 attributes2[],
		std::vector<uint16_t>& outIndices,
		std::vector<Attribute0>& outAttributes0,
		std::vector<Attribute1>& outAttributes1,
		std::vector<Attribute2>& outAttributes2)
{
	if(indices0 && indices1 && indices2)
	{
#if 0 // Recursion test
		std::vector<uint16_t> indices01;
		std::vector<Attribute0> attributes010;
		std::vector<Attribute1> attributes011;

		SeparateToUnifiedIndices(
					numIndices,
					indices0,
					indices1,
					numAttributes0, attributes0,
					numAttributes1, attributes1,
					indices01,
					attributes010,
					attributes011);
		assert(indices01.size() == numIndices);
		assert(attributes010.size() == attributes011.size());
		std::vector<std::pair<Attribute0, Attribute1>> attributes01(attributes010.size());
		for(size_t i = 0; i < attributes011.size(); i++)
			attributes01[i] = std::make_pair(attributes010[i], attributes011[i]);

		std::vector<std::pair<Attribute0, Attribute1>> outAttributes01;
		SeparateToUnifiedIndices(
					numIndices,
					indices01.data(),
					indices2,
					attributes01.size(), attributes01.data(),
					numAttributes2, attributes2,
					outIndices,
					outAttributes01,
					outAttributes2);
		outAttributes0.reserve(outAttributes01.size());
		outAttributes1.reserve(outAttributes01.size());
		for(auto attr: outAttributes01)
		{
			outAttributes0.push_back(attr.first);
			outAttributes1.push_back(attr.second);
		}
#else
		std::unordered_map<uint64_t, uint16_t> vertexMap;

		for(size_t i = 0; i < numIndices; ++i)
		{
			uint16_t index0 = indices0[i];
			uint16_t index1 = indices1[i];
			uint16_t index2 = indices2[i];

			uint64_t combinedIndex = (uint64_t(index2) << 32) | (uint64_t(index1) << 16) | index0;
			auto it = vertexMap.find(combinedIndex);

			if(it == vertexMap.end())
			{
				uint16_t outIndex = outAttributes0.size();
				vertexMap[combinedIndex] = outIndex;
				outIndices.push_back(outIndex);
				assert(numAttributes0 > index0);
				outAttributes0.push_back(attributes0[index0]);

				assert(numAttributes1 > index1);
				outAttributes1.push_back(attributes1[index1]);

				assert(numAttributes2 > index2);
				outAttributes2.push_back(attributes2[index2]);
			}
			else
				outIndices.push_back(it->second);
		}
#endif
	}
	else if(!indices0)
	{
		SeparateToUnifiedIndices(
				numIndices,
				indices1,
				indices2,
				numAttributes1, attributes1,
				numAttributes2, attributes2,
				outIndices,
				outAttributes1,
				outAttributes2);
	}
	else if(!indices1)
	{
		SeparateToUnifiedIndices(
				numIndices,
				indices0,
				indices2,
				numAttributes0, attributes0,
				numAttributes2, attributes2,
				outIndices,
				outAttributes0,
				outAttributes2);
	}
	else if(!indices2)
	{
		SeparateToUnifiedIndices(
				numIndices,
				indices0,
				indices1,
				numAttributes0, attributes0,
				numAttributes1, attributes1,
				outIndices,
				outAttributes0,
				outAttributes1);
	}
}

}

#endif // MESHUTILS_H
