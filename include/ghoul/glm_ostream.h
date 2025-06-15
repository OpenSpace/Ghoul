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

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_common.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

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

namespace std {
// adding operator overloads for MacOS instead of std::format overloads

// for glm::vec2, glm::vec3, glm::vec4
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

// For glm::mat types, implement similarly
template<typename T, glm::qualifier Q>
ostream& operator<<(ostream& os, const glm::mat<4, 4, T, Q>& m) {
    for (int i = 0; i < 4; ++i) {
        os << '|';
        for (int j = 0; j < 4; ++j) {
            os << m[j][i];
            if (j < 3) os << ", ";
        }
        os << '|';
        if (i < 3) os << '\n';
    }
    return os;
}
}

#endif // __GHOUL___GLM_OSTREAM___H__
