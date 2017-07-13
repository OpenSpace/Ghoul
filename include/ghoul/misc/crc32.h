/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 *****************************************************************************************
 * This file is inspired by Peter Kankowski (kankowski@narod.ru) and his work available  *
 * at http://www.strchr.com/hash_functions and http://www.strchr.com/crc32_popcnt        *
 *****************************************************************************************
 * Slicing-by-8 algorithms by Michael E. Kounavis and Frank L. Berry from Intel Corp.    *
 * http://www.intel.com/technology/comms/perfnet/download/CRC_generators.pdf             *
 ****************************************************************************************/

#ifndef __GHOUL___CRC32___H__
#define __GHOUL___CRC32___H__

#include <string>

namespace ghoul {

/**
 * Computes the CRC-32 hash of the zero terminated string \p s If the string is longer
 * than <code>len</code>, the behavior is undefined. If the passed value \p s is a compile
 * constant, the crc32 hash will also be computed at compile time.
 * \param s The string for which to compute the CRC-32 hash
 * \return The hash value for the passed string
 */
constexpr unsigned int hashCRC32(const char* s);

/**
 * Computes the CRC-32 hash of the string \p s.
 * \param s The string for which to compute the CRC-32 hash
 * \return The hash value for the passed string
 */
unsigned int hashCRC32(const std::string& s);

/**
 * A postfix operator that will convert a string into a crc32 at compile time. This is
 * functionally equivalent to calling #hashCRC32 with the string \p s.
 * \param s The character array that is converted
 * \param len The length of the character array
 * \return The CRC32 hash of \p s
 */
constexpr unsigned int operator "" _crc32(const char* s, size_t len);

} // namespace ghoul

#include "crc32.inl"

#endif // __GHOUL___CRC32___H__
