/*	Hash.h

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


#ifndef HASH_H
#define HASH_H

#include <string>
#include <cstring>
#include <cstdint>

typedef uint32_t Hash;

/// Compile-time MurmurHash3
/** Only use Murmur::Hash(), the rest is for internal use only. */
namespace Murmur
{
	constexpr uint32_t RotateL(uint32_t x, int8_t r) { return (x << r) | (x >> (32 - r)); }

	const uint32_t mixC1 = 0xcc9e2d51;
	const uint32_t mixC2 = 0x1b873593;
	const uint32_t mixR1 = 15;
	const uint32_t mixR2 = 13;
	const uint32_t m1 = 5;
	const uint32_t m2 = 0xe6546b64;

	constexpr uint32_t Mix2(uint32_t k)
	{
		return RotateL(k * mixC1, mixR1) * mixC2;
	}

	constexpr uint32_t Mix(uint32_t block, uint32_t state)
	{
		return RotateL(state ^ Mix2(block), mixR2) * m1 + m2;
	}

	constexpr uint32_t ShiftAndXor(uint32_t val, int shift)
	{
		return val ^ (val >> shift);
	}

	/// Convert char[4] to uint32_t
	/** Little endian. */
	constexpr uint32_t ToInt32(const char* bytes)
	{
		return (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
	}

	constexpr uint32_t Hash2(const char* str, size_t length, uint32_t state)
	{
		return
			length == 0 ? state :
			length == 1 ? Mix2(str[0]) ^ state :
			length == 2 ? Mix2((str[1] << 8) | str[0]) ^ state :
			length == 3 ? Mix2((str[2] << 16) | (str[1] << 8) | str[0]) ^ state :
			/*else*/ Hash2(str + 4, length - 4, Mix(ToInt32(str), state));
	}

	const uint32_t c1 = 0x85ebca6b;
	const uint32_t c2 = 0xc2b2ae35;
	const uint32_t r1 = 16;
	const uint32_t r2 = 13;
	const uint32_t r3 = 16;

	/// Finalize step of MurmurHash3
	constexpr uint32_t Finalize(uint32_t h0)
	{
		return ShiftAndXor(ShiftAndXor(ShiftAndXor(h0, r1) * c1, r2) * c2, r3);
	}

	/// Compute Hash of string
	constexpr Hash Hash(const char* str, size_t length, uint32_t seed = 42)
	{
		return Finalize(Hash2(str, length, seed) ^ uint32_t(length));
	}
}

constexpr uint32_t operator"" _H(const char* str, size_t length)
{
	return Murmur::Hash(str, length);
}

namespace HashUtils
{
	constexpr Hash MakeHash(const char* str, size_t length)
	{
		return Murmur::Hash(str, length);
	}

	inline Hash MakeHash(const std::string& string) {return MakeHash(string.data(), string.size());}
	inline Hash MakeHash(const char* string) {return MakeHash(string, strlen(string));}
	constexpr Hash MakeHash(const char* begin, const char* end)
	{
		return MakeHash(begin, end - begin);
	}

	/** Bitwise OR. */
	constexpr Hash Combine(Hash hash1, Hash hash2) {return hash1 ^ hash2;}

	template<size_t size>
	constexpr Hash H(const char (&string)[size]) { return Murmur::Hash(string, size - 1); }
}

#endif // HASH_H
