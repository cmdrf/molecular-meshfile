/*	TextStream.h

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

#ifndef TEXTSTREAM_H
#define TEXTSTREAM_H

#include <stdexcept>
#include <stdint.h>

class TextReadStreamBase
{
public:
	virtual ~TextReadStreamBase() {}
	virtual char* GetNextLine() = 0;
};

template<class Storage>
class TextReadStream : public TextReadStreamBase
{
public:
	TextReadStream(Storage& storage) : mStorage(storage)
	{
		mLine[0] = 0;
	}

	/// Reads a line and returns it
	/** @returns Pointer to an internal storage of the line, which is only
			valid until the next call to GetNextLine(). Returns nullptr if the
			storage is at its end. */
	char* GetNextLine() override
	{
		if(mStorage.EndOfData())
			return nullptr;

		char c;
		int i;
		for(i = 0; i < kMaxLineLength; ++i)
		{
			if(mStorage.EndOfData() || mStorage.Read(&c, 1) != 1 || c == '\n')
			{
				mLine[i] = 0;
				return mLine;
			}
			else
				mLine[i] = c;
		}
		throw std::range_error("Line too long");
	}

private:
	static const int kMaxLineLength = 1024;
	char mLine[kMaxLineLength];
	Storage& mStorage;
};

#endif // TEXTSTREAM_H
