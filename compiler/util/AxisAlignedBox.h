/*	AxisAlignedBox.h

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

#ifndef MOLECULAR_AXISALIGNEDBOX_H
#define MOLECULAR_AXISALIGNEDBOX_H

#undef min
#undef max

#include "Vector3.h"
#include <cassert>
#include <limits>
#include <stdexcept>

namespace molecular
{

/// Axis aligned (bounding) box (AABB)
class AxisAlignedBox
{
public:
	inline AxisAlignedBox();

	inline void Stretch(const Vector3& vec);

	const Vector3& GetMin() const {return mMin;}
	const Vector3& GetMax() const {return mMax;}

private:
	Vector3 mMin, mMax;
};

std::ostream& operator<<(std::ostream& o, const AxisAlignedBox& box);

inline AxisAlignedBox::AxisAlignedBox() :
	mMin(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()),
	mMax(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity())
{

}

inline void AxisAlignedBox::Stretch(const Vector3& vec)
{
	for(int i = 0; i < 3; ++i)
	{
		if(vec[i] < mMin[i])
			mMin[i] = vec[i];

		if(vec[i] > mMax[i])
			mMax[i] = vec[i];
	}
}

} // namespace molecular

#endif // MOLECULAR_AXISALIGNEDBOX_H
