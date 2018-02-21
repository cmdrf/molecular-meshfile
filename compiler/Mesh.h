/*	Mesh.h

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

#ifndef MESH_H
#define MESH_H

#include <vector>
#include "util/Hash.h"
#include "BufferInfo.h"
#include "util/Vector3.h"
#include "util/Vector4.h"
#include <unordered_map>
#include <cassert>

using Hash = uint32_t;

template<class T>
struct AttributeTraits {};

template<>
struct AttributeTraits<float>
{
	static const VertexAttributeInfo::Type type = VertexAttributeInfo::kFloat;
	static const unsigned int components = 1;
};

template<>
struct AttributeTraits<Vector2>
{
	static const VertexAttributeInfo::Type type = VertexAttributeInfo::kFloat;
	static const unsigned int components = 2;
};

template<>
struct AttributeTraits<Vector3>
{
	static const VertexAttributeInfo::Type type = VertexAttributeInfo::kFloat;
	static const unsigned int components = 3;
};

template<>
struct AttributeTraits<Vector4>
{
	static const VertexAttributeInfo::Type type = VertexAttributeInfo::kFloat;
	static const unsigned int components = 4;
};

template<>
struct AttributeTraits<IntVector4>
{
	static const VertexAttributeInfo::Type type = VertexAttributeInfo::kInt32;
	static const unsigned int components = 4;
};

/// Intermediate representation of mesh data
class Mesh
{
public:
	class Attribute
	{
		friend class Mesh;
	public:
		template<typename T>
		const T* GetData() const
		{
			assert(mNumComponents == AttributeTraits<T>::components);
			assert(mType == AttributeTraits<T>::type);
			return static_cast<const T*>(static_cast<const void*>(mData.data()));
		}

		template<typename T>
		T* GetData()
		{
			assert(mNumComponents == AttributeTraits<T>::components);
			assert(mType == AttributeTraits<T>::type);
			return static_cast<T*>(static_cast<void*>(mData.data()));
		}

		/// Set attribute data from raw data
		void SetData(VertexAttributeInfo::Type type, unsigned int components, const void* data, size_t size)
		{
			mType = type;
			mNumComponents = components;
			auto begin = static_cast<const uint8_t*>(data);
			mData.assign(begin, begin + size);
		}

		const void* GetRawData() const {return mData.data();}
		size_t GetRawSize() const {return mData.size();}

		VertexAttributeInfo::Type GetType() const {return mType;}
		unsigned int GetNumComponents() const {return mNumComponents;}

	private:
		VertexAttributeInfo::Type mType;
		unsigned int mNumComponents;
		std::vector<uint8_t> mData;
	};

	explicit Mesh(unsigned int numVertices, IndexBufferInfo::Mode mode = IndexBufferInfo::Mode::kTriangles) :
	    mNumVertices(numVertices),
	mMode(mode)
	{}
	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) = default;

	template<typename T>
	void SetAttributeData(Hash name, const T* data, size_t count)
	{
		assert(count == mNumVertices);
		Attribute attr;
		attr.mType = AttributeTraits<T>::type;
		attr.mNumComponents = AttributeTraits<T>::components;
		auto begin = static_cast<const uint8_t*>(static_cast<const void*>(data));
		attr.mData.assign(begin, begin + count * sizeof(T));
		mAttributes.emplace(name, std::move(attr));
	}

	/// Set attribute data from raw data
	void SetAttributeData(Hash name, VertexAttributeInfo::Type type, unsigned int components, const void* data, size_t size)
	{
		Attribute attr;
		attr.mType = type;
		attr.mNumComponents = components;
		auto begin = static_cast<const uint8_t*>(data);
		attr.mData.assign(begin, begin + size);
		mAttributes.emplace(name, std::move(attr));
	}

	std::vector<uint32_t>& GetIndices() {return mIndices;}
	const std::vector<uint32_t>& GetIndices() const {return mIndices;}

	IndexBufferInfo::Mode GetMode() const {return mMode;}
	void SetMode(IndexBufferInfo::Mode mode) {mMode = mode;}

	const std::string& GetMaterial() const {return mMaterial;}
	void SetMaterial(const std::string& material) {mMaterial = material;}

	unsigned int GetNumVertices() const {return mNumVertices;}

	const std::unordered_map<Hash, Attribute>& GetAttributes() const {return mAttributes;}
	std::unordered_map<Hash, Attribute>& GetAttributes() {return mAttributes;}
	const Attribute& GetAttribute(Hash name) const {return mAttributes.at(name);}
	Attribute& GetAttribute(Hash name) {return mAttributes.at(name);}
	void RemoveAttribute(Hash name) {mAttributes.erase(name);}

private:
	std::vector<uint32_t> mIndices;
	unsigned int mNumVertices;
	IndexBufferInfo::Mode mMode;
	std::string mMaterial;
	std::unordered_map<Hash, Attribute> mAttributes;
};

using MeshSet = std::vector<Mesh>;

#endif // MESH_H
