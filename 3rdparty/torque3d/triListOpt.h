//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef TRI_LIST_OPT_H_
#define TRI_LIST_OPT_H_

#include <cstdint>
#include <vector>

namespace TriListOpt
{
typedef uint32_t IndexType;

const uint32_t MaxSizeVertexCache = 32;

struct VertData
{
	int32_t cachePosition = -1;
	float score = 0.0f;
	uint32_t numReferences = 0;
	uint32_t numUnaddedReferences = 0;
	std::vector<int32_t> triIndex;
};

struct TriData
{
	bool isInList = false;
	float score = 0.0f;
	uint32_t vertIdx[3] = {0, 0, 0};
};

class LRUCacheModel
{
	struct LRUCacheEntry
	{
		LRUCacheEntry *next = nullptr;
		uint32_t vIdx = 0;
		VertData *vData = nullptr;
	};

	LRUCacheEntry *mCacheHead = nullptr;

public:
	~LRUCacheModel();
	void enforceSize(const std::size_t maxSize, std::vector<uint32_t> &outTrisToUpdate);
	void useVertex(const uint32_t vIdx, VertData *vData);
	int32_t getCachePosition(const uint32_t vIdx);
};


/// This method will look at the index buffer for a triangle list, and generate
/// a new index buffer which is optimized using Tom Forsyth's paper:
/// "Linear-Speed Vertex Cache Optimization"
/// http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html
/// @param   numVerts Number of vertices indexed by the 'indices'
/// @param numIndices Number of elements in both 'indices' and 'outIndices'
/// @param    indices Input index buffer
/// @param outIndices Output index buffer
///
/// @note Both 'indices' and 'outIndices' can point to the same memory.
void OptimizeTriangleOrdering(const std::size_t numVerts, const std::size_t numIndices, const uint32_t indices[], IndexType outIndices[]);

namespace FindVertexScore
{
    const float CacheDecayPower = 1.5f;
	const float LastTriScore = 0.75f;
	const float ValenceBoostScale = 2.0f;
	const float ValenceBoostPower = 0.5f;

	float score(const VertData &vertexData);
};

}

#endif
