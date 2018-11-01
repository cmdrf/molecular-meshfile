/*	FileStreamStorage.cpp

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

#include "FileStreamStorage.h"

namespace molecular
{

FileReadStorage::FileReadStorage(FileReadStorage&& that) noexcept :
	mFile(that.mFile)
{
	that.mFile = nullptr;
}

FileReadStorage::FileReadStorage(const char* filename)
{
#ifdef __STDC_LIB_EXT1__
	errno_t errno = fopen_s(&mFile, filename, "rb");
#else
	mFile = fopen(filename, "rb");
#endif
	if(!mFile)
		throw std::runtime_error(std::string(filename) + " could not be opened for reading: " + StringUtils::StrError(errno));
}

FileReadStorage::FileReadStorage(const std::string& filename) :
	FileReadStorage(filename.c_str())
{
}


FileReadStorage::~FileReadStorage()
{
	if(mFile)
		fclose(mFile);
}

FileReadStorage& FileReadStorage::operator=(FileReadStorage&& that)
{
	if(mFile)
		fclose(mFile);
	mFile = that.mFile;
	that.mFile = nullptr;
	return *this;
}

FileWriteStorage::FileWriteStorage(const char* filename)
{
#ifdef __STDC_LIB_EXT1__
	errno_t error = fopen_s(&mFile, filename, "wb");
#else
	mFile = fopen(filename, "wb");
#endif
	if(!mFile)
		throw std::runtime_error(std::string(filename) + " could not be opened for writing: " + StringUtils::StrError(errno));
}

FileWriteStorage::FileWriteStorage(const std::string& filename) :
	FileWriteStorage(filename.c_str())
{
}

FileWriteStorage::~FileWriteStorage()
{
	assert(mFile);
	fclose(mFile);
}

} // namespace molecular
