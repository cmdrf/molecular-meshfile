/*	MeshCompiler.h

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

#ifndef MOLECULAR_MESHCOMPILER_H
#define MOLECULAR_MESHCOMPILER_H

#include <molecular/util/StreamStorage.h>
#include <molecular/meshfile/MeshFile.h>
#include "ObjFile.h"
#include <molecular/util/Mesh.h>
#include <molecular/util/BufferInfo.h>

namespace molecular
{

/// Functions for writing optimized mesh files
namespace MeshCompiler
{

/// Write mesh file from a set of buffers and data specifications
void Compile(
		const std::vector<std::pair<const void*, size_t>>& vertexBuffers,
		const std::vector<std::pair<const void*, size_t>>& indexBuffers,
		const std::vector<std::vector<VertexAttributeInfo>>& vertexDataSets,
		const std::vector<unsigned int>& vertexDataSetVertexCounts,
		const std::vector<IndexBufferInfo>& indexSpecs,
		const float boundsMin[3], const float boundsMax[3],
		WriteStorage& storage
		);

/// Compile OBJ file
/** @deprecated Use ObjFileToMeshSet first. */
void Compile(ObjFile& objFile, WriteStorage& storage);

util::MeshSet ObjFileToMeshSet(ObjFile& objFile);

/** @todo Optimize vertex buffer layout. */
void Compile(const util::MeshSet& meshes, WriteStorage& storage);

}

} // namespace molecular

#endif // MOLECULAR_MESHCOMPILER_H

