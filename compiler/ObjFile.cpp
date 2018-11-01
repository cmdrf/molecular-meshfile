/*	ObjFile.cpp

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

#include "ObjFile.h"
#include "util/StringUtils.h"
#include <cstring>

namespace molecular
{

ObjFile::ObjFile(TextReadStreamBase& stream, float scale) :
	mScale(scale)
{
	const char* line;
	std::string currentMaterial;
	bool newMaterial = false;
	while((line = stream.GetNextLine()))
	{
		if(line[0] == 0 || line[0] == '#')
			continue; // Skip empty lines and comments

		if(line[0] == 'v')
		{
			if(line[1] == ' ')
			{
				Vector3 v;
				int result = StringUtils::ScanF(line, "v %f %f %f", &v[0], &v[1], &v[2]);
				if(result != 3)
					continue;
				v *= mScale;
				mVertices.push_back(v);
				mBoundingBox.Stretch(v);
			}
			else if(line[1] == 't')
			{
				float u, v;
				int result = StringUtils::ScanF(line, "vt %f %f", &u, &v);
				if(result != 2)
					continue;
				mTexCoords.push_back(Vector2(u, 1 - v));
			}
			else if(line[1] == 'n')
			{
				float x, y, z;
				int result = StringUtils::ScanF(line, "vn %f %f %f", &x, &y, &z);
				if(result != 3)
					continue;
				mNormals.push_back(Vector3(x, y, z));
			}
		}
		else if(line[0] == 'g')
		{
			NewVertexGroup(line + 2, currentMaterial);
			newMaterial = false;
		}
		else if(line[0] == 'f')
		{
			/* Blender changes materials without creating vertex groups, so
				create vertex group if material changed: */
			if(newMaterial)
			{
				NewVertexGroup(currentMaterial, currentMaterial);
				newMaterial = false;
			}

			if(mVertexGroups.empty())
				throw std::runtime_error("Face definition without vertex group");

			int v[4] = {0, 0, 0, 0};
			int t[4] = {0, 0, 0, 0};
			int n[4] = {0, 0, 0, 0};

			bool isTri = false;
			bool isQuad = false;
			// Positions and texCoords:
			int result = StringUtils::ScanF(line, "f %d/%d %d/%d %d/%d %d/%d", &v[0], &t[0], &v[1], &t[1], &v[2], &t[2], &v[3], &t[3]);
			if(result == 6 || result == 8)
			{
				isTri = (result == 6);
				isQuad = (result == 8);
				mVertexGroups.back().hasNormals = false;
				mVertexGroups.back().hasTexCoords = true;
			}
			else
			{
				// Positions, texCoords and normals:
				result = StringUtils::ScanF(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &v[0], &t[0], &n[0], &v[1], &t[1], &n[1], &v[2], &t[2], &n[2], &v[3], &t[3], &n[3]);
				if(result == 9 || result == 12)
				{
					isTri = (result == 9);
					isQuad = (result == 12);
					mVertexGroups.back().hasNormals = true;
					mVertexGroups.back().hasTexCoords = true;
				}
				else
				{
					// Positions and normals:
					result = StringUtils::ScanF(line, "f %d//%d %d//%d %d//%d %d//%d", &v[0], &n[0], &v[1], &n[1], &v[2], &n[2], &v[3], &n[3]);
					if(result == 6 || result == 8)
					{
						isTri = (result == 6);
						isQuad = (result == 8);
						mVertexGroups.back().hasNormals = true;
						mVertexGroups.back().hasTexCoords = false;
					}
					else
					{
						// Only positions:
						result = StringUtils::ScanF(line, "f %d %d %d %d", &v[0], &v[1], &v[2], &v[3]);
						if(result == 3)
							isTri = true;
						else if(result == 4)
							isQuad = true;
						mVertexGroups.back().hasNormals = false;
						mVertexGroups.back().hasTexCoords = false;
					}
				}
			}

			// Indices count from 1!
			if(v[0] > mVertices.size() || v[1] > mVertices.size() || v[2] > mVertices.size())
				throw std::runtime_error("ObjFile: Non-existent vertex referenced.");

			if(isTri)
			{
				mTriangles.push_back(Triangle(v, t, n));
				mVertexGroups.back().numTriangles++;
			}
			else if(isQuad)
			{
				if(v[3] > mVertices.size())
					throw std::runtime_error("ObjFile: Non-existent vertex referenced.");

				mQuads.push_back(Quad(v, t, n));
				mVertexGroups.back().numQuads++;
			}
		}
		else if(!strncmp(line, "usemtl", 6))
		{
			const char* material = line + 7; // Skip "usemtl "
			if(!mVertexGroups.empty() && mVertexGroups.back().material.empty())
				mVertexGroups.back().material = material;
			else
			{
				currentMaterial = material;
				newMaterial = true;
			}
		}
		else if(!strncmp(line, "mtllib", 6))
		{
			mMtlLibFiles.push_back(line + 7);
		}
		else if(line[0] == 'o')
		{
			NewVertexGroup(line + 2, "");
		}
	}
}


void ObjFile::CalculateNormals()
{
	assert(!mVertices.empty());

	mNormals.clear();
	mNormals.resize(mVertices.size(), Vector3(0.0, 0.0, 0.0));

	std::vector<uint8_t> normalDenoms(mNormals.size() / 3, 0);

	for(auto q: mQuads)
	{
		Vector3 p0 = GetVertex(q, 0);
		Vector3 p1 = GetVertex(q, 1);
		Vector3 p2 = GetVertex(q, 2);
		Vector3 p3 = GetVertex(q, 3);

		AddNormal(p0, p1, p3, mNormals[q.vertexIndices[0]]);
		AddNormal(p1, p2, p0, mNormals[q.vertexIndices[1]]);
		AddNormal(p2, p3, p1, mNormals[q.vertexIndices[2]]);
		AddNormal(p3, p0, p2, mNormals[q.vertexIndices[3]]);

		for(int i = 0; i < 4; ++i)
		{
			normalDenoms[q.vertexIndices[i]] += 1;
			q.normalIndices[i] = q.vertexIndices[i];
		}
	}

	for(auto& t: mTriangles)
	{
		Vector3 p0 = GetVertex(t, 0);
		Vector3 p1 = GetVertex(t, 1);
		Vector3 p2 = GetVertex(t, 2);
		Vector3 v1 = p1 - p0;
		Vector3 v2 = p2 - p0;

		Vector3 n = v1.CrossProduct(v2);
		n.SetLength(1.0);
		for(int i = 0; i < 3; ++i)
		{
			mNormals[t.vertexIndices[i]] += n;
			normalDenoms[t.vertexIndices[i]] += 1;
			t.normalIndices[i] = t.vertexIndices[i];
		}
	}

	for(size_t i = 0; i < normalDenoms.size(); i++)
	{
		mNormals[i] /= normalDenoms[i];
	}
}

void ObjFile::NewVertexGroup(const std::string& name, const std::string& material)
{
	VertexGroup group;
	group.name = name;
	group.firstQuad = mQuads.size();
	group.numQuads = 0;
	group.firstTriangle = mTriangles.size();
	group.numTriangles = 0;
	group.material = material;
	mVertexGroups.push_back(group);
}

} // namespace molecular
