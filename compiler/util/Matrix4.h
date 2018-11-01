/*	Matrix4.h

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

#ifndef MOLECULAR_MATRIX4_H
#define MOLECULAR_MATRIX4_H

#include "Matrix.h"
#include "Vector4.h"

namespace molecular
{

/// A 4x4 matrix
class Matrix4 : public Matrix<4, 4>
{
public:
	Matrix4()
	{
		*this = Identity();
	}

	Matrix4(const Matrix<4, 4>& mat) : Matrix<4, 4>(mat) {}

	inline Vector4 operator*(const Vector4& v) const;
	using Matrix<4,4>::operator*;
};

inline Vector4 Matrix4::operator*(const Vector4& v) const
{
	Matrix<4, 1> in(v);
	Matrix<4, 1> out = *this * in;
	return Vector4(out(0, 0), out(1, 0), out(2, 0), out(3, 0));
}

} // namespace molecular

#endif // MOLECULAR_MATRIX4_H
