/*	ColladaToMesh.h

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

#ifndef MOLECULAR_COLLADATOMESH_H
#define MOLECULAR_COLLADATOMESH_H

#include "ColladaFile.h"
#include <molecular/util/Mesh.h>

namespace molecular
{

/// Functions to extract meshes and skin data from COLLADA files
namespace ColladaToMesh
{

Mesh ToMesh(const ColladaFile& file, const ColladaFile::Mesh& mesh);

MeshSet ToMesh(const ColladaFile& file, const ColladaFile::Node& node);
MeshSet ToMesh(const ColladaFile& file, const ColladaFile::VisualScene& scene);
MeshSet ToMesh(const ColladaFile& file);

/// Returns inverse bind matrices of the first skin controller found in the file
/** Sorted as in CharacterAnimation.
	@returns Bind pose matrices, usually CharacterAnimation::kBoneCount elements. */
std::vector<Matrix4> ToBindMatrices(const ColladaFile& file);

void ReadInverseBindMatrices(const ColladaFile::Skin& skin,
		std::vector<Hash>& jointNames,
		std::vector<Matrix4>& inverseBindMatrices);
void ReadVertexWeights(const ColladaFile::Skin& skin,
		std::vector<Vector4>& outWeights,
		std::vector<IntVector4>& outJointIndices);
}

} // namespace molecular

#endif // MOLECULAR_COLLADATOMESH_H
