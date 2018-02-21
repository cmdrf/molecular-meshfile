/*	Vector.h

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

#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>
#include <iostream>

/// Vector base class template
/** Cannot be instanciated directly. Use typedef or subclassing, or the Vector
	template. */
template<int components, class TSubclass, typename _T = float>
class VectorTmpl
{
public:
	typedef _T T;
	using Subclass = TSubclass;

	VectorTmpl()
	{
		for(int i = 0; i < components; ++i)
			v[i] = 0;
	}

	// Accessors:
	T operator[](const int n) const {return v[n];}
	T& operator[](const int n) {return v[n];}
	operator const T*() const {return v;}
	operator T*() {return v;}

	/// Component-wise multiplication
	Subclass operator*(T r) const
	{
		Subclass newVec;
		for(int i = 0; i < components; ++i)
			newVec[i] = v[i] * r;
		return newVec;
	}

	/// Component-wise multiplication
	Subclass operator*(const VectorTmpl& in) const
	{
		Subclass newVec;
		for(int i = 0; i < components; ++i)
			newVec[i] = v[i] * in[i];
		return newVec;
	}

	Subclass& operator+=(const VectorTmpl& in)
	{
		for(int i = 0; i < components; ++i)
			v[i] += in[i];
		return *static_cast<Subclass*>(this);
	}

	/// Component-wise in-place multiplication
	Subclass& operator*=(T r)
	{
		for(int i = 0; i < components; ++i)
			v[i] *= r;
		return *static_cast<Subclass*>(this);
	}

	/// Component-wise in-place multiplication
	Subclass& operator*=(const VectorTmpl& in)
	{
		for(int i = 0; i < components; ++i)
			v[i] *= in[i];
		return *static_cast<Subclass*>(this);
	}

	/// Component-wise scalar division
	Subclass operator/(T r) const
	{
		Subclass newVec;
		for(int i = 0; i < components; ++i)
			newVec[i] = v[i] / r;
		return newVec;
	}

	/// Component-wise in-place scalar division
	Subclass& operator/=(T r)
	{
		for(int i = 0; i < components; ++i)
			v[i] /= r;
		return *static_cast<Subclass*>(this);
	}

	/// Component-wise in-place division
	Subclass& operator/=(const VectorTmpl& in)
	{
		for(int i = 0; i < components; ++i)
			v[i] /= in[i];
		return *static_cast<Subclass*>(this);
	}

	Subclass operator-(const VectorTmpl& in) const
	{
		Subclass newVec;
		for(int i = 0; i < components; ++i)
			newVec[i] = v[i] - in[i];
		return newVec;
	}

//	/// Unary minus
	Subclass operator-() const
	{
		Subclass newVec;
		for(int i = 0; i < components; ++i)
			newVec[i] = -v[i];
		return newVec;
	}

	Subclass operator+(const VectorTmpl& in) const
	{
		Subclass newVec;
		for(int i = 0; i < components; ++i)
			newVec[i] = v[i] + in[i];
		return newVec;
	}

	bool operator==(const VectorTmpl& in) const
	{
		for(int i = 0; i < components; ++i)
		{
			if(v[i] != in[i])
				return false;
		}
		return true;
	}

	T LengthSquared() const
	{
		T l = 0;
		for(int i = 0; i < components; ++i)
			l += v[i] * v[i];
		return l;
	}

	T Length() const {return std::sqrt(LengthSquared());}

	Subclass Normalized() const {return *this / Length();}

	/// Dot product
	T Dot(const VectorTmpl& in)
	{
		T sum = 0;
		for(int i = 0; i < components; ++i)
			sum += v[i] * in[i];
		return sum;
	}

protected:
	T v[components];
};

/// Vector template with adjustable dimensionality
template<int dim, class T = float>
class Vector : public VectorTmpl<dim, Vector<dim, T>, T>
{
public:
	Vector() = default;
	Vector(std::initializer_list<T> l)
	{
		assert(l.size() == dim);
		for(int i = 0; i < dim; ++i)
			this->v[i] = l.begin()[i];
	}
};

/// Two-dimensional vector
class Vector2 : public VectorTmpl<2, Vector2>
{
public:
	Vector2() = default;
	Vector2(float x, float y) {v[0] = x; v[1] = y;}
};

/// Integer vector with two components
class IntVector2 : public VectorTmpl<2, IntVector2, int32_t>
{
public:
	IntVector2() = default;
	IntVector2(int32_t x, int32_t y)
	{
		v[0] = x;
		v[1] = y;
	}
};

/// Integer vector with three components
class IntVector3 : public VectorTmpl<3, IntVector3, int32_t>
{
public:
	IntVector3() = default;
	IntVector3(int32_t x, int32_t y, int32_t z)
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
	}
};

/// Integer vector with four components
class IntVector4 : public VectorTmpl<4, IntVector4, int32_t>
{
public:
	IntVector4() = default;
	IntVector4(int32_t x, int32_t y, int32_t z, int32_t w)
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
		v[3] = w;
	}
};

/// Unsigned integer vector with two components
class UIntVector2 : public VectorTmpl<2, UIntVector2, uint32_t>
{
public:
	UIntVector2() = default;
	UIntVector2(uint32_t x, uint32_t y)
	{
		v[0] = x;
		v[1] = y;
	}
};

/// Unsigned integer vector with three components
class UIntVector3 : public VectorTmpl<3, UIntVector3, uint32_t>
{
public:
	UIntVector3() = default;
	UIntVector3(uint32_t x, uint32_t y, uint32_t z)
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
	}
};

/// Unsigned integer vector with four components
class UIntVector4 : public VectorTmpl<4, UIntVector4, uint32_t>
{
public:
	UIntVector4() = default;
	UIntVector4(uint32_t x, uint32_t y, uint32_t z, uint32_t w)
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
		v[3] = w;
	}

	UIntVector4(const UIntVector2& iv, uint32_t z, uint32_t w)
	{
		v[0] = iv[0];
		v[1] = iv[1];
		v[2] = z;
		v[3] = w;
	}

	UIntVector4(const UIntVector3& iv, uint32_t w)
	{
		v[0] = iv[0];
		v[1] = iv[1];
		v[2] = iv[2];
		v[3] = w;
	}
};

/** Does not work for Vector2, Vector3 etc. */
template<int dim, class T>
inline std::ostream& operator<<(std::ostream& stream, const Vector<dim, T>& vec)
{
	stream << "(" << vec[0];
	for(int i = 1; i < dim; i++)
		stream << ", " << vec[i];
	stream << ")";
	return stream;
}

/// Stream operator for Vector2
inline std::ostream& operator<<(std::ostream& stream, const Vector2& vec)
{
	stream << "(" << vec[0] << ", " << vec[1] << ")";
	return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const IntVector4& vec)
{
	stream << "(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ", " << vec[3] << ")";
	return stream;
}

#endif // VECTOR_H
