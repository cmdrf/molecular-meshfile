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

#include "triListOpt.h"
#include <cassert>
#include <cmath>

namespace TriListOpt
{

void OptimizeTriangleOrdering(const std::size_t numVerts, const std::size_t numIndices, const uint32_t indices[], IndexType outIndices[])
{
	if(numVerts == 0 || numIndices == 0)
	{
		std::copy(indices, indices + numIndices, outIndices);
		return;
	}

	assert(numIndices % 3 == 0 && "Number of indicies not divisible by 3, not a good triangle list.");
	const std::size_t numPrimitives = numIndices / 3;

	//
	// Step 1: Run through the data, and initialize
	//
	std::vector<VertData> vertexData(numVerts);
	std::vector<TriData> triangleData(numPrimitives);

	uint32_t curIdx = 0;
	for(unsigned int tri = 0; tri < numPrimitives; tri++)
	{
		TriData &curTri = triangleData[tri];

		for(int c = 0; c < 3; c++)
		{
			const uint32_t& curVIdx = indices[curIdx];
			assert(curVIdx < numVerts && "Out of range index.");

			// Add this vert to the list of verts that define the triangle
			curTri.vertIdx[c] = curVIdx;

			VertData &curVert = vertexData[curVIdx];

			// Increment the number of triangles that reference this vertex
			curVert.numUnaddedReferences++;

			curIdx++;
		}
	}

	// Allocate per-vertex triangle lists, and calculate the starting score of
	// each of the verts
	for(unsigned int v = 0; v < numVerts; v++)
	{
		VertData& curVert = vertexData[v];
		curVert.triIndex.resize(curVert.numUnaddedReferences);
		curVert.score = FindVertexScore::score(curVert);
	}

	// These variables will be used later, but need to be declared now
	int32_t nextNextBestTriIdx = -1, nextBestTriIdx = -1;
	float nextNextBestTriScore = -1.0f, nextBestTriScore = -1.0f;

#define VALIDATE_TRI_IDX(idx) if(idx > -1) { assert(idx < numPrimitives && "Out of range triangle index."); assert(!triangleData[idx].isInList && "Triangle already in list, bad."); }
#define CHECK_NEXT_NEXT_BEST(score, idx) { if(score > nextNextBestTriScore) { nextNextBestTriIdx = idx; nextNextBestTriScore = score; } }
#define CHECK_NEXT_BEST(score, idx) { if(score > nextBestTriScore) { CHECK_NEXT_NEXT_BEST(nextBestTriScore, nextBestTriIdx); nextBestTriIdx = idx; nextBestTriScore = score; } VALIDATE_TRI_IDX(nextBestTriIdx); }

	// Fill-in per-vertex triangle lists, and sum the scores of each vertex used
	// per-triangle, to get the starting triangle score
	curIdx = 0;
	for(unsigned int tri = 0; tri < numPrimitives; tri++)
	{
		TriData& curTri = triangleData[tri];

		for(int c = 0; c < 3; c++)
		{
			const uint32_t& curVIdx = indices[curIdx];
			assert(curVIdx < numVerts && "Out of range index.");
			VertData &curVert = vertexData[curVIdx];

			// Add triangle to triangle list
			curVert.triIndex[curVert.numReferences++] = tri;

			// Add vertex score to triangle score
			curTri.score += curVert.score;

			curIdx++;
		}

		// This will pick the first triangle to add to the list in 'Step 2'
		CHECK_NEXT_BEST(curTri.score, tri);
		CHECK_NEXT_NEXT_BEST(curTri.score, tri);
	}

	//
	// Step 2: Start emitting triangles...this is the emit loop
	//
	LRUCacheModel lruCache;
	for(unsigned int outIdx = 0; outIdx < numIndices; /* this space intentionally left blank */ )
	{
		// If there is no next best triangle, than search for the next highest
		// scored triangle that isn't in the list already
		if(nextBestTriIdx < 0)
		{
			// TODO: Something better than linear performance here...
			nextBestTriScore = nextNextBestTriScore = -1.0f;
			nextBestTriIdx = nextNextBestTriIdx = -1;

			for(unsigned int tri = 0; tri < numPrimitives; tri++)
			{
				TriData &curTri = triangleData[tri];

				if(!curTri.isInList)
				{
					CHECK_NEXT_BEST(curTri.score, tri);
					CHECK_NEXT_NEXT_BEST(curTri.score, tri);
				}
			}
		}
		assert(nextBestTriIdx > -1 && "Ran out of 'nextBestTriangle' before I ran out of indices...not good.");

		// Emit the next best triangle
		TriData &nextBestTri = triangleData[nextBestTriIdx];
		assert(!nextBestTri.isInList && "Next best triangle already in list, this is no good.");
		for(int i = 0; i < 3; i++)
		{
			// Emit index
			outIndices[outIdx++] = IndexType(nextBestTri.vertIdx[i]);

			// Update the list of triangles on the vert
			VertData &curVert = vertexData[nextBestTri.vertIdx[i]];
			curVert.numUnaddedReferences--;
			for(unsigned int t = 0; t < curVert.numReferences; t++)
			{
				if(curVert.triIndex[t] == nextBestTriIdx)
				{
					curVert.triIndex[t] = -1;
					break;
				}
			}

			// Update cache
			lruCache.useVertex(nextBestTri.vertIdx[i], &curVert);
		}
		nextBestTri.isInList = true;

		// Enforce cache size, this will update the cache position of all verts
		// still in the cache. It will also update the score of the verts in the
		// cache, and give back a list of triangle indicies that need updating.
		std::vector<uint32_t> trisToUpdate;
		lruCache.enforceSize(MaxSizeVertexCache, trisToUpdate);

		// Now update scores for triangles that need updates, and find the new best
		// triangle score/index
		nextBestTriIdx = -1;
		nextBestTriScore = -1.0f;
		for(std::vector<uint32_t>::iterator itr = trisToUpdate.begin(); itr != trisToUpdate.end(); itr++)
		{
			TriData& tri = triangleData[*itr];

			// If this triangle isn't already emitted, re-score it
			if(!tri.isInList)
			{
				tri.score = 0.0f;

				for(int i = 0; i < 3; i++)
					tri.score += vertexData[tri.vertIdx[i]].score;

				CHECK_NEXT_BEST(tri.score, *itr);
				CHECK_NEXT_NEXT_BEST(tri.score, *itr);
			}
		}

		// If there was no love finding a good triangle, than see if there is a
		// next-next-best triangle, and if there isn't one of those...well than
		// I guess we have to find one next time
		if(nextBestTriIdx < 0 && nextNextBestTriIdx > -1)
		{
			if(!triangleData[nextNextBestTriIdx].isInList)
			{
				nextBestTriIdx = nextNextBestTriIdx;
//				nextBestTriScore = nextNextBestTriScore; // Never read
				VALIDATE_TRI_IDX(nextNextBestTriIdx);
			}

			// Nuke the next-next best
			nextNextBestTriIdx = -1;
			nextNextBestTriScore = -1.0f;
		}

		// Validate triangle we are marking as next-best
		VALIDATE_TRI_IDX(nextBestTriIdx);
	}

#undef CHECK_NEXT_BEST
#undef CHECK_NEXT_NEXT_BEST
#undef VALIDATE_TRI_IDX
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

LRUCacheModel::~LRUCacheModel()
{
	for( LRUCacheEntry* entry = mCacheHead; entry != nullptr; )
	{
		LRUCacheEntry* next = entry->next;
		delete entry;
		entry = next;
	}
}

//------------------------------------------------------------------------------

void LRUCacheModel::useVertex(const uint32_t vIdx, VertData *vData)
{
	LRUCacheEntry *search = mCacheHead;
	LRUCacheEntry *last = nullptr;

	while(search != nullptr)
	{
		if(search->vIdx == vIdx)
			break;

		last = search;
		search = search->next;
	}

	// If this vertex wasn't found in the cache, create a new entry
	if(search == nullptr)
	{
		search = new LRUCacheEntry;
		search->vIdx = vIdx;
		search->vData = vData;
	}

	if(search != mCacheHead)
	{
		// Unlink the entry from the linked list
		if(last)
			last->next = search->next;

		// Vertex that got passed in is now at the head of the cache
		search->next = mCacheHead;
		mCacheHead = search;
	}
}

//------------------------------------------------------------------------------

void LRUCacheModel::enforceSize(const std::size_t /*maxSize*/, std::vector<uint32_t> &outTrisToUpdate)
{
	// Clear list of triangles to update scores for
	outTrisToUpdate.clear();

	int32_t length = 0;
	LRUCacheEntry* next = mCacheHead;
	LRUCacheEntry* last = nullptr;

	// Run through list, up to the max size
	while(next != nullptr && length < MaxSizeVertexCache)
	{
		VertData& vData = *next->vData;

		// Update cache position on verts still in cache
		vData.cachePosition = length++;

		for(unsigned int i = 0; i < vData.numReferences; i++)
		{
			const int32_t triIdx = vData.triIndex[i];
			if(triIdx > -1)
			{
				unsigned int j = 0;
				for(; j < outTrisToUpdate.size(); j++)
					if(outTrisToUpdate[j] == triIdx)
						break;
				if(j == outTrisToUpdate.size())
					outTrisToUpdate.push_back(triIdx);
			}
		}

		// Update score
		vData.score = FindVertexScore::score(vData);

		last = next;
		next = next->next;
	}

	assert(last);
	// NULL out the pointer to the next entry on the last valid entry
	last->next = nullptr;

	// If next != nullptr, than we need to prune entries from the tail of the cache
	while(next != nullptr)
	{
		// Update cache position on verts which are going to get tossed from cache
		next->vData->cachePosition = -1;

		LRUCacheEntry *curEntry = next;
		next = next->next;

		delete curEntry;
	}
}

//------------------------------------------------------------------------------

int32_t LRUCacheModel::getCachePosition(const uint32_t vIdx)
{
	int32_t length = 0;
	LRUCacheEntry *next = mCacheHead;
	while(next != nullptr)
	{
		if(next->vIdx == vIdx)
			return length;

		length++;
		next = next->next;
	}

	return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html
namespace FindVertexScore
{

float score(const VertData &vertexData)
{
	// If nobody needs this vertex, return -1.0
	if(vertexData.numUnaddedReferences < 1)
		return -1.0f;

	float score = 0.0f;

	if(vertexData.cachePosition < 0)
	{
		// Vertex is not in FIFO cache - no score.
	}
	else
	{
		if(vertexData.cachePosition < 3)
		{
			// This vertex was used in the last triangle,
			// so it has a fixed score, whichever of the three
			// it's in. Otherwise, you can get very different
			// answers depending on whether you add
			// the triangle 1,2,3 or 3,1,2 - which is silly.
			score = FindVertexScore::LastTriScore;
		}
		else
		{
			assert(vertexData.cachePosition < MaxSizeVertexCache && "Out of range cache position for vertex");

			// Points for being high in the cache.
			const float Scaler = 1.0f / (MaxSizeVertexCache - 3);
			score = 1.0f - (vertexData.cachePosition - 3) * Scaler;
			score = std::pow(score, FindVertexScore::CacheDecayPower);
		}
	}

	// Bonus points for having a low number of tris still to
	// use the vert, so we get rid of lone verts quickly.
	float ValenceBoost = std::pow(vertexData.numUnaddedReferences, -FindVertexScore::ValenceBoostPower);
	score += FindVertexScore::ValenceBoostScale * ValenceBoost;

	return score;
}

} // namspace FindVertexScore

} // namespace TriListOpt
