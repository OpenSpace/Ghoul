/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#ifndef __GHOUL___STRINGCONVERSION___H__
#define __GHOUL___STRINGCONVERSION___H__

#include <string>

namespace ghoul {

/**
 * Converts the passed \p string into a \c T value and returns it. For each valid
 * conversion, a template specialization has to be created. This function is meant to be
 * analogous to the <code>std::to_string</code> function and should behave as such:
 *
 * <code>ghoul::to_string(ghoul::from_string(s)) == s</code>
 *
 * <code>ghoul::from_string(ghoul::to_string(v)) == v</code>
 */
template <typename T>
T from_string(const std::string& string) {
    // Unfortunately, we can't write 'false' here, as the compiler is a bit too eager to
    // evaluate that
    static_assert(sizeof(T) == -1, "Missing from_string implementation");
}

/**
 * Converts the passed \p value to its string representation. The default implementation
 * calls the <code>std::to_string</code> function. User-defined types are supported by
 * creating a specialization of this function.
 */
template <typename T>
std::string to_string(const T& value) {
    // std::string does not define the identity transformation so we have to handle that
    if constexpr (std::is_same_v<T, std::string>) {
        return value;
    }
    else {
        return std::to_string(value);
    }
}

} // namespace ghoul

#endif // __GHOUL___FROMSTRING___H__
