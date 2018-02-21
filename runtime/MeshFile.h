/*	MeshFile.h

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

#ifndef MESHFILE_H
#define MESHFILE_H

#include <cstdint>
#include <cassert>
#include "BufferInfo.h"
#include <iostream>

/// File structure for meshes
/** Cast your file contents to this to access mesh data.
	@see MeshCompiler */
struct MeshFile
{
	struct Buffer
	{
		enum class Type : uint32_t
		{
			kVertex = 1,
			kIndex
		};

		Type type;
		uint32_t offset;
		uint32_t size;
		uint32_t reserved;
	};
	static_assert(sizeof(Buffer) == 16, "Buffer struct not aligned correctly");

	struct VertexDataSet
	{
		uint32_t numVertexSpecs;
		uint32_t vertexSpecsOffset;
		uint32_t numVertices;
		uint32_t reserved;
	};
	static_assert(sizeof(VertexDataSet) == 16, "VertexDataSet struct not aligned correctly");

	static const uint32_t kMagic = 0x8e8e54f1;
	static const uint32_t kVersion = 1;

	uint32_t magic; ///< File identification magic value
	uint32_t version; ///< Version of the file format this file was written for
	uint32_t reserved;
	uint32_t numBuffers; ///< Number of buffers (vertex and index buffers combined)
	uint32_t numVertexDataSets; ///< Number of vertex data sets @see VertexDataSet
	uint32_t numIndexSpecs; ///< Number of index specifications @see IndexBufferInfo
	uint32_t vertexDataSetsOffset; ///< Byte offset inside file to vertex data sets
	uint32_t indexSpecsOffset; ///< Byte offset inside file to index specifications
	float boundsMin[3]; ///< Axis aligned bounding box minimum @see AxisAlignedBox
	float boundsMax[3]; ///< Axis aligned bounding box maximum @see AxisAlignedBox

	Buffer buffers[0];

	const VertexDataSet& GetVertexDataSet(unsigned int i) const
	{
		assert(i < numVertexDataSets);
		return reinterpret_cast<const VertexDataSet*>(reinterpret_cast<const char*>(this) + vertexDataSetsOffset)[i];
	}

	const VertexAttributeInfo& GetVertexSpec(unsigned int dataSet, unsigned int spec) const
	{
		const VertexDataSet& set = GetVertexDataSet(dataSet);
		assert(spec < set.numVertexSpecs);
		return reinterpret_cast<const VertexAttributeInfo*>(reinterpret_cast<const char*>(this) + set.vertexSpecsOffset)[spec];
	}

	const IndexBufferInfo& GetIndexSpec(unsigned int i) const
	{
		assert(i < numIndexSpecs);
		return reinterpret_cast<const IndexBufferInfo*>(reinterpret_cast<const char*>(this) + indexSpecsOffset)[i];
	}

	const void* GetBufferData(unsigned int i) const
	{
		assert(i < numBuffers);
		return reinterpret_cast<const char*>(this) + buffers[i].offset;
	}
};

static_assert(sizeof(MeshFile) == 56, "Unexpected size for MeshFile");

/** For unit test and mesh info tool. */
inline std::ostream& operator<<(std::ostream& o, MeshFile::Buffer::Type type)
{
	switch(type)
	{
	case MeshFile::Buffer::Type::kIndex:
		o << "kIndex";
		break;
	case MeshFile::Buffer::Type::kVertex:
		o << "kVertex";
		break;
	}
	return o;
}

#endif // MESHFILE_H

