/*	StreamStorage.h

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

#ifndef MOLECULAR_STREAMSTORAGE_H
#define MOLECULAR_STREAMSTORAGE_H

#include <stdint.h> // uint8_t
#include <stdlib.h> // size_t

namespace molecular
{

/// Base class for data storage to be read from
/** Data storage can be be files, memory blocks and the like. */
class ReadStorage
{
public:
	virtual ~ReadStorage();
	virtual size_t Read(void* ptr, const size_t size) = 0;
	virtual bool EndOfData() = 0;
};

/// Base class for data storage to be written to
/** Data storage can be be files, memory blocks and the like. */
class WriteStorage
{
public:
	virtual ~WriteStorage() {}

	/// Write an array.
	virtual void Write(const void* ptr, const size_t size) = 0;
};

/// WriteStorage whose cursor can be set randomly
class RandomAccessWriteStorage : public WriteStorage
{
public:
	virtual size_t GetCursor() const = 0;
	virtual void SetCursor(size_t cursor) = 0;
};

} // namespace molecular

#endif
