/*	FloatToHalf.cpp

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

#include "FloatToHalf.h"

FloatToHalf::FloatToHalf()
{
	for(int i0 = 0; i0 < 256; ++i0)
	{
		int i1 = i0 + 256;
		int exponent = i0 - 127;
		if(exponent < -24)
		{
			mShiftTable[i0] = 24;
			mShiftTable[i1] = 24;
			mBaseTable[i0] = 0x0000;
			mBaseTable[i1] = 0x8000;
		}
		else if(exponent < -14)
		{
			mShiftTable[i0] = -exponent - 1;
			mShiftTable[i1] = -exponent - 1;
			mBaseTable[i0] = (0x0400 >> (-exponent - 14));
			mBaseTable[i1] = (0x0400 >> (-exponent - 14)) | 0x8000;
		}
		else if(exponent < 16)
		{
			mShiftTable[i0] = 13;
			mShiftTable[i1] = 13;
			mBaseTable[i0] = ((exponent + 15) << 10);
			mBaseTable[i1] = ((exponent + 15) << 10) | 0x8000;
		}
		else if(exponent < 128)
		{
			mShiftTable[i0] = 24;
			mShiftTable[i1] = 24;
			mBaseTable[i0] = 0x7c00;
			mBaseTable[i1] = 0xfc00;
		}
		else
		{
			mShiftTable[i0] = 13;
			mShiftTable[i1] = 13;
			mBaseTable[i0] = 0x7c00;
			mBaseTable[i1] = 0xfc00;
		}
	}
}
