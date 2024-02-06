/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2024                                                               *
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

#ifndef __GHOUL___CONSTMAP___H__
#define __GHOUL___CONSTMAP___H__

#include <optional>

template <typename Key, typename Value, int Size>
struct Map {
    std::array<std::pair<Key, Value>, Size> data;

    [[ nodiscard ]] constexpr std::optional<Value> at(const Key& key) const {
        // @CPP20
        // manual constexpr std::find_if implementation until C++20 is available
        for (const std::pair<const Key, Value>& d : data) {
            if (d.first == key) {
                return d.second;
            }
        }

        return std::nullopt;
    }

    [[ nodiscard ]] constexpr bool contains(const Key& key) const {
        // @CPP20
        // manual implementation of std::contains implementation until C++20 is available
        for (const std::pair<const Key, Value>& d : data) {
            if (d.first == key) {
                return true;
            }
        }
        return false;
    }
};


#endif // __GHOUL___CONSTMAP___H__
