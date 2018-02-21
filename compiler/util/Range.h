/*	Range.h

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


#ifndef RANGE_H
#define RANGE_H

/// Pair of iterators
/** Can be used in range-based for loops. */
template<class T>
class Range
{
public:
	Range(T begin, T end) : mBegin(begin), mEnd(end) {}

	T begin() {return mBegin;}
	T end() {return mEnd;}

private:
	T mBegin, mEnd;
};

template<class T>
Range<T> MakeRange(T begin, T end)
{
	return Range<T>(begin, end);
}

template<class T>
Range<typename T::iterator> MakeRange(T container)
{
	return Range<typename T::iterator>(begin(container), end(container));
}

#endif // RANGE_H

