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

#ifndef __GHOUL___GLM_OSTREAM___H__
#define __GHOUL___GLM_OSTREAM___H__
#include <ghoul/format.h>
#include <ghoul/misc/stringconversion.h>
#include <ostream>

#ifndef GLM_META_PROG_HELPERS
#define GLM_META_PROG_HELPERS
#endif // GLM_META_PROG_HELPERS

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif // GLM_ENABLE_EXPERIMENTAL

#ifndef __APPLE__
#ifndef GLM_FORCE_CTOR_INIT
#define GLM_FORCE_CTOR_INIT
#endif // GLM_FORCE_CTOR_INIT
#endif

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_common.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

#include <sstream>
#include <optional>

namespace std {
// adding operator overloads for MacOS instead of std::format overloads

// for glm::vec2, glm::vec3, glm::vec4
/*
template<typename T, glm::qualifier Q>
ostream& operator<<(ostream& os, const glm::vec<2, T, Q>& v) {
    return os << '(' << v.x << ", " << v.y << ')';
}

template<typename T, glm::qualifier Q>
ostream& operator<<(ostream& os, const glm::vec<3, T, Q>& v) {
    return os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
}

template<typename T, glm::qualifier Q>
ostream& operator<<(ostream& os, const glm::vec<4, T, Q>& v) {
    return os << '(' << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ')';
}
*/

// Generic operator<< for all glm::vec types to match std::formatter
template <glm::length_t L, typename T, glm::qualifier Q>
ostream& operator<<(ostream& os, const glm::vec<L, T, Q>& v) {
    os << '{';
    for (glm::length_t i = 0; i < L; ++i) {
        os << v[i];
        if (i < L - 1) {
            os << ',';
        }
    }
    os << '}';
    return os;
}

// For glm::mat types, implement similarly

template<typename T, glm::length_t C, glm::length_t R, glm::qualifier Q>
ostream& operator<<(ostream& os, const glm::mat<C, R, T, Q>& mat) {
    os << "{";
    for (glm::length_t i = 0; i < C; ++i) {
        for (glm::length_t j = 0; j < R; ++j) {
            os << mat[i][j];
            // Print comma except after the last element
            if (!(i == C - 1 && j == R - 1)) {
                os << ",";
            }
        }
    }
    os << "}";
    return os;
}

} // namespace std

namespace ghoul {

// there is a similar template in misc/stringconversion.h
// XCode needs a specific template for std::optional

template <typename T>
std::string to_string(const std::optional<T>& v) {
    if (!v) return "null";
    else {
        T temp = *v;
        return std::to_string(temp);
}

} // namespace ghoul

#endif // __GHOUL___GLM_OSTREAM___H__
