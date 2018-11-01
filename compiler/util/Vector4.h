/* Vector4.h
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

#ifndef MOLECULAR_VECTOR4_H
#define MOLECULAR_VECTOR4_H

#include "Vector3.h"

namespace molecular
{

/// Four-dimensional vector
class Vector4 : public VectorTmpl<4, Vector4>
{
public:
	Vector4() {}
	Vector4(T x, T y, T z, T w = 1) {v[0] = x; v[1] = y; v[2] = z; v[3] = w;}
	Vector4(const Vector3& xyz, T w = 1) {v[0] = xyz[0]; v[1] = xyz[1]; v[2] = xyz[2]; v[3] = w;}
};

inline std::ostream& operator<<(std::ostream& stream, const Vector4& vec)
{
	stream << "(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ", " << vec[3] << ")";
	return stream;
}

} // namespace molecular

#endif // MOLECULAR_VECTOR4_H
