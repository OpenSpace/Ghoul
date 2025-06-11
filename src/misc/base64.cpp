/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2025                                                               *
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
 ****************************************************************************************/

#include <ghoul/misc/base64.h>

namespace ghoul {

std::vector<uint8_t> decodeBase64(const std::string_view base64) {
    // Implementation of this function based on:
    // 1. https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base
    //    64-with-cpp/
    // 2. https://stackoverflow.com/a/180949

    constexpr std::string_view base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    auto isBase64 = [](unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    };

    int inLen = base64.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    std::array<unsigned char, 4> charArray4;
    std::array<unsigned char, 3> charArray3;
    std::vector<uint8_t> ret;

    while (inLen-- && ( base64[in_] != '=') && isBase64(base64[in_])) {
        charArray4[i++] = base64[in_];
        in_++;

        if (i == 4) {
            for (i = 0; i < 4; i++) {
                charArray4[i] = base64Chars.find(charArray4[i]);
            }

            charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
            charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
            charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

            for (i = 0; (i < 3); i++) {
                ret.push_back(charArray3[i]);
            }
            i = 0;
        }
    }

    if (i != 0) {
        for (j = i; j < 4; j++) {
            charArray4[j] = 0;
        }

        for (j = 0; j < 4; j++) {
            charArray4[j] = base64Chars.find(charArray4[j]);
        }

        charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
        charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
        charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

        for (j = 0; (j < i - 1); j++) {
            ret.push_back(charArray3[j]);
        }
    }

    return ret;
}

} // namespace ghoul
