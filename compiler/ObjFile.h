/*	ObjFile.h

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

#ifndef MOLECULAR_OBJFILE_H
#define MOLECULAR_OBJFILE_H

#include <molecular/util/Vector3.h>
#include <molecular/util/AxisAlignedBox.h>
#include <molecular/util/TextStream.h>
#include <vector>
#include <array>

namespace molecular
{

/// Reads 3D models in .obj text files
class ObjFile
{
public:
	explicit ObjFile(TextReadStreamBase& stream, float scale = 1.0f);

	/** Call after applying morph targets. */
	void CalculateNormals();

	/// Face structure templated over the number of vertices
	/** A face consists usually of three or four vertices. This structure
		contains only indices to vertices in mVertices, mTexCoords and
		mNormals */
	template<int vertices>
	struct Face
	{
		Face(){}
		Face(int v[vertices], int t[vertices], int n[vertices])
		{
			for(int i = 0; i < vertices; ++i)
			{
				// OBJ files count from 1. Careful with the initial value 0 for v, t and n!
				vertexIndices[i]   = v[i] - 1;
				texCoordIndices[i] = t[i] - 1;
				normalIndices[i]   = n[i] - 1;
			}
		}

		std::array<uint16_t, vertices> vertexIndices;
		std::array<uint16_t, vertices> texCoordIndices;
		std::array<uint16_t, vertices> normalIndices;
	};

	typedef Face<4> Quad; ///< Face with four vertices (quad)
	typedef Face<3> Triangle; ///< Face with three vertices (triangle)

	/// Submesh
	struct VertexGroup
	{
		std::string name;
		unsigned int firstQuad; ///< Index to entry in mQuads
		unsigned int numQuads;
		unsigned int firstTriangle; ///< Index to entry in mTriangles
		unsigned int numTriangles;
		std::string material;
		bool hasNormals;
		bool hasTexCoords;
	};

	inline const std::vector<Vector3>& GetVertices() const {return mVertices;}
	inline const std::vector<Vector2>& GetTexCoords() const {return mTexCoords;}
	inline const std::vector<Quad>& GetQuads() const {return mQuads;}
	inline const std::vector<Triangle>& GetTriangles() const {return mTriangles;}
	inline const std::vector<Vector3>& GetNormals() const {return mNormals;}
	inline const std::vector<VertexGroup>& GetVertexGroups() const {return mVertexGroups;}

//	/** @returns True if there are normals either loaded from the file or
//			generated via CalculateNormals. */
//	bool HasNormals() const {return !mNormals.empty();}
//	bool HasTexCoords() const {return !mTexCoords.empty();}

	const util::AxisAlignedBox& GetBoundingBox() const {return mBoundingBox;}

protected:
	/// Vertex coordinates
	std::vector<Vector3> mVertices;
	std::vector<Vector2> mTexCoords;

	/** This can be populated by CalculateNormals(). After normal calculation, this
		vector has the same size as mVertices. */
	std::vector<Vector3> mNormals;

	std::vector<Quad> mQuads;
	std::vector<Triangle> mTriangles;
	std::vector<VertexGroup> mVertexGroups;
	std::vector<std::string> mMtlLibFiles;

	template<int vertices>
	Vector3 GetVertex(const Face<vertices>& face, int i)
	{
		assert(mVertices.size() > face.vertexIndices[i]);
		return mVertices[face.vertexIndices[i]];
	}

	/** Used by CalculateNormals. */
	inline void AddNormal(const Vector3& p0, const Vector3& p1, const Vector3& p2, Vector3& outNormal)
	{
		Vector3 v1 = p1 - p0;
		Vector3 v2 = p2 - p0;

		Vector3 n = v1.CrossProduct(v2);
		n.SetLength(1.0);

		outNormal += n;
	}

	void NewVertexGroup(const std::string& name, const std::string& material);

	util::AxisAlignedBox mBoundingBox;
	float mScale;

};

} // namespace molecular

#endif // MOLECULAR_OBJFILE_H
