/*	FileStreamStorage.h

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

#ifndef FILESTREAMSTORAGE_H
#define FILESTREAMSTORAGE_H

#include <cstdio>
#include <string>
#include <cstring>
#include <errno.h>
#include "StreamStorage.h"
#include "StringUtils.h"
#include <stdexcept>
#include <assert.h>
#include <iostream>

/// A derived ReadStorage for reading regular files
class FileReadStorage : public ReadStorage
{
public:
	/// Deleted copy constructor
	FileReadStorage(const FileReadStorage& that) = delete;

	/// Move constructor
	FileReadStorage(FileReadStorage&& that) noexcept;

	/// Construct from file name
	explicit FileReadStorage(const char* filename);

	/// Construct from file name (std::string)
	explicit FileReadStorage(const std::string& filename);

	/// Destructor
	~FileReadStorage();

	/// Deleted copy assignment
	FileReadStorage& operator=(const FileReadStorage& that) = delete;

	/// Move assignment
	FileReadStorage& operator=(FileReadStorage&& that);

	inline size_t Read(void* ptr, const size_t size) override
	{
		assert(ptr);
		assert(mFile);
		return fread(ptr, 1, size, mFile);
	}

	inline void Skip(size_t size)
	{
		assert(mFile);
		fseek(mFile, long(size), SEEK_CUR);
	}

	bool EndOfData() override {assert(mFile); return feof(mFile) != 0;}

	size_t GetSize()
	{
		assert(mFile);
		long oldPos = ftell(mFile);
		fseek(mFile, 0, SEEK_END);
		long endPos = ftell(mFile);
		fseek(mFile, oldPos, SEEK_SET);
		return endPos;
	}

	size_t GetCursor() {return ftell(mFile);}

private:
	FILE* mFile = nullptr;
};
	
/// A derived WriteStorage for writing regular files
class FileWriteStorage : public RandomAccessWriteStorage
{
public:
	explicit FileWriteStorage(const char* filename);
	explicit FileWriteStorage(const std::string& filename);
	FileWriteStorage(const FileWriteStorage&) = delete;
	~FileWriteStorage();

	FileWriteStorage& operator=(const FileWriteStorage&) = delete;

	inline void Write(const void* ptr, const size_t size) override
	{
		assert(mFile);
		assert(ptr);
		fwrite(ptr, 1, size, mFile);
	}

	size_t GetCursor() const override
	{
		assert(mFile);
		return ftell(mFile);
	}

	void SetCursor(size_t cursor) override
	{
		assert(mFile);
		if(fseek(mFile, static_cast<long>(cursor), SEEK_SET) != 0)
			throw std::runtime_error(std::string("fseek: ") + StringUtils::StrError(errno));
	}

private:
	FILE* mFile = nullptr;
};


#endif
