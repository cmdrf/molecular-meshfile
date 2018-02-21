/*	BufferInfo.h

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

#ifndef BUFFERINFO_H
#define BUFFERINFO_H

#include <string>
#include <ostream>

/// Information about vertex attribute data in a buffer
struct VertexAttributeInfo
{
	enum
	{
		kPosition = 0xe7dccea2,
		kNormal = 0x11c36e42,
		kTextureCoords = 0x7dd0ace6,
		kVertexPrt0 = 0xc03b4aec,
		kVertexPrt1 = 0xa2739294,
		kVertexPrt2 = 0x9d2051c8,
		kSkinWeights = 0xfc228c1f,
		kSkinJoints = 0xe2cf8e75,
		kUnknown = 0
	};

	enum Type
	{
		kFloat = 1,
		kInt8,
		kUInt8,
		kInt16,
		kUInt16,
		kInt32,
		kUInt32,
		kHalf
	};

	VertexAttributeInfo() :
		semantic(kPosition), type(kFloat), components(3), offset(0), stride(0), buffer(0), normalized(true)
	{

	}

	VertexAttributeInfo(Type type, int components, int offset, int stride, int buffer, bool normalized = true) :
		semantic(kUnknown),
		type(type),
		components(components),
		offset(offset),
		stride(stride),
		buffer(buffer),
		normalized(normalized)
	{}

	uint32_t semantic;

	/// Data type in buffer
	Type type;

	/// Number of components per entry
	int components;

	/// Offset to the first element in the buffer, measured in bytes
	int offset;

	/// Offset from one entry to the next
	/** Measured in bytes. 0 means entries are tightly packed. */
	int stride;

	/// Index of the buffer containing the data
	int buffer;

	bool normalized;
};

/// Information about index data in a buffer
/** This structure is directly serialized to and deserialized from mesh files. */
struct IndexBufferInfo
{
	IndexBufferInfo() : mode(Mode::kTriangles), type(Type::kUInt16), buffer(0), offset(0), count(0), vertexDataSet(0), material{0} {}

	enum class Mode : uint32_t
	{
		kPoints = 1,
		kTriangles,
		kLines,
		kTriangleFan,
		kTriangleStrip,
		kLineStrip,
		kLineStripAdjacency,
		kTrianglesAdjacency,
		kTriangleStripAdjacency
	};

	enum class Type : uint32_t
	{
		kUInt8 = 1,
		kUInt16,
		kUInt32
	};

	Mode mode;
	Type type;

	/// Index of the buffer containing the data
	uint32_t buffer;

	/// Offset to the first element in the buffer, measured in bytes
	uint32_t offset;

	/// Number of indices
	uint32_t count;
	uint32_t vertexDataSet;

	char material[32];
};

inline std::ostream& operator<<(std::ostream& o, IndexBufferInfo::Mode mode)
{
	switch(mode)
	{
	case IndexBufferInfo::Mode::kPoints: return (o << "kPoints");
	case IndexBufferInfo::Mode::kTriangles: return (o << "kTriangles");
	case IndexBufferInfo::Mode::kLines: return (o << "kLines");
	case IndexBufferInfo::Mode::kTriangleFan: return (o << "kTriangleFan");
	case IndexBufferInfo::Mode::kTriangleStrip: return (o << "kTriangleStrip");
	case IndexBufferInfo::Mode::kLineStrip: return (o << "kLineStrip");
	case IndexBufferInfo::Mode::kLineStripAdjacency: return (o << "kLineStripAdjacency");
	case IndexBufferInfo::Mode::kTrianglesAdjacency: return (o << "kTrianglesAdjacency");
	case IndexBufferInfo::Mode::kTriangleStripAdjacency: return (o << "kTriangleStripAdjacency");
	}
}

inline std::ostream& operator<<(std::ostream& o, IndexBufferInfo::Type type)
{
	switch(type)
	{
	case IndexBufferInfo::Type::kUInt8: return (o << "kUInt8");
	case IndexBufferInfo::Type::kUInt16: return (o << "kUInt16");
	case IndexBufferInfo::Type::kUInt32: return (o << "kUInt32");
	}
}

inline std::ostream& operator<<(std::ostream& o, VertexAttributeInfo::Type type)
{
	switch(type)
	{
	case VertexAttributeInfo::Type::kFloat: return (o << "kFloat");
	case VertexAttributeInfo::Type::kInt8: return (o << "kInt8");
	case VertexAttributeInfo::Type::kUInt8: return (o << "kUInt8");
	case VertexAttributeInfo::Type::kInt16: return (o << "kInt16");
	case VertexAttributeInfo::Type::kUInt16: return (o << "kUInt16");
	case VertexAttributeInfo::Type::kInt32: return (o << "kInt32");
	case VertexAttributeInfo::Type::kUInt32: return (o << "kUInt32");
	case VertexAttributeInfo::Type::kHalf: return (o << "kHalf");
	}
}

#endif // BUFFERINFO_H
