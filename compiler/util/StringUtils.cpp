/*	StringUtils.cpp

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

#include "StringUtils.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#ifndef _MSC_VER
#include <libgen.h>
#endif

#if __APPLE__
#include <xlocale.h>
#endif

namespace molecular
{

namespace StringUtils
{

int ScanF(const char* string, const char* format, ...)
{
	va_list args;
	va_start(args, format);
#if __APPLE__
	int result = vsscanf_l(string, 0, format, args);
#else
	int result = vsscanf(string, format, args);
#endif
	va_end(args);
	return result;
}

bool EndsWith(const char* haystack, const char* needle)
{
	size_t haystackLength = strlen(haystack);
	size_t needleLength = strlen(needle);
	if(needleLength > haystackLength)
		return false;

	return strncmp(haystack + (haystackLength - needleLength), needle, needleLength) == 0;
}

bool EndsWith(const std::string& haystack, const std::string& needle)
{
	size_t haystackLength = haystack.length();
	size_t needleLength = needle.length();
	if(needleLength > haystackLength)
		return false;
	return !haystack.compare(haystackLength - needleLength, needleLength, needle);
}

std::string StrError(int errnum)
{
#ifdef __STDC_LIB_EXT1__
	char buffer[256] = {0};
	strerror_s(buffer, errnum);
	return buffer;
#else
	return strerror(errnum);
#endif
}

}

} // namespace molecular
