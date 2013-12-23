/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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

#include "misc/crc32.h"
#include <cassert>

#define CRCPOLY 0x82f63b78 // reversed 0x1EDC6F41
#define CRCINIT 0xFFFFFFFF

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

namespace ghoul {

namespace {
    unsigned int _crcLookup[8][256];
    bool _isInitialized = false;

    void initializeLookupTable() {
        assert(!_isInitialized);
        for (unsigned int i = 0; i <= 0xFF; i++) {
            unsigned int x = i;
            for (unsigned int j = 0; j < 8; j++)
                x = (x>>1) ^ (CRCPOLY & (-(int)(x & 1)));
            _crcLookup[0][i] = x;
        }
        
        for (unsigned int i = 0; i <= 0xFF; i++) {
            unsigned int c = _crcLookup[0][i];
            for (unsigned int j = 1; j < 8; j++) {
                c = _crcLookup[0][c & 0xFF] ^ (c >> 8);
                _crcLookup[j][i] = c;
            }
        }
        
        _isInitialized = true;
    }
}
    // dword unsigned long
    // res unsigned int

unsigned int hashCRC32(const char* s, size_t len) {
    if (!_isInitialized)
        initializeLookupTable();

    unsigned int crc = 0xFFFFFFFF;
    // Align to DWORD boundary
    size_t align = (sizeof(unsigned long) - (size_t)s) & (sizeof(unsigned long) - 1);
    align = min(align, len);
    len -= align;
    for (; align; align--)
        crc = _crcLookup[0][(crc ^ *s++) & 0xFF] ^ (crc >> 8);

#if 0
    // TODO: reliable check for architecture needed here
    // hardware acceleration only available on newer Core i5/i7
    SIZE_T ndwords = len / sizeof(unsigned long);
    for (; ndwords; ndwords--) {
        crc = _mm_crc32_u32(crc, *(unsigned long*)s);
        s += sizeof(unsigned long);
    }

    len &= sizeof(unsigned long) - 1;
    for (; len; len--)
        crc = _mm_crc32_u8(crc, *s++);
#else
    // Slicing-by-8
    size_t nqwords = len / (sizeof(unsigned int) + sizeof(unsigned int));
    for (; nqwords; nqwords--) {
        crc ^= *(unsigned int*)s;
        s += sizeof(unsigned int);
        unsigned int next = *(unsigned int*)s;
        s += sizeof(unsigned int);
        crc =
            _crcLookup[7][(crc      ) & 0xFF] ^
            _crcLookup[6][(crc >>  8) & 0xFF] ^
            _crcLookup[5][(crc >> 16) & 0xFF] ^
            _crcLookup[4][(crc >> 24)] ^
            _crcLookup[3][(next      ) & 0xFF] ^
            _crcLookup[2][(next >>  8) & 0xFF] ^
            _crcLookup[1][(next >> 16) & 0xFF] ^
            _crcLookup[0][(next >> 24)];
    }

    len &= sizeof(unsigned int) * 2 - 1;
    for (; len; len--)
        crc = _crcLookup[0][(crc ^ *s++) & 0xFF] ^ (crc >> 8);

#endif
    return ~crc;
}

unsigned int hashCRC32(const std::string& s) {
    return hashCRC32(s.c_str(), s.length());
}

} // namespace ghoul
