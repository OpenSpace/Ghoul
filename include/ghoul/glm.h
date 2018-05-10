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

#ifndef __GHOUL___GLM___H__
#define __GHOUL___GLM___H__

#ifdef __APPLE__
// The GLM header throw 'anonymous struct' warnings that we do not want. By marking the
// following files as system headers, all warnings are ignored
#pragma clang diagnostic push
#pragma clang system_header
#endif // __APPLE__


#ifdef __unix__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif // __unix__


#ifndef GLM_META_PROG_HELPERS
#define GLM_META_PROG_HELPERS
#endif // GLM_META_PROG_HELPERS

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif // GLM_ENABLE_EXPERIMENTAL

// Enabling swizzling breaks binary compatibility between glm::vecX and float* ?
// #ifndef GLM_SWIZZLE
// #define GLM_SWIZZLE
// #endif // GLM_SWIZZLE

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>

#include <string>

namespace glm {

template <typename genType>
GLM_FUNC_QUALIFIER GLM_CONSTEXPR genType tau()
{
    return genType(6.28318530717958647692528676655900576);
}

} // namespace glm


namespace ghoul {


template <typename T>
struct glm_components : public std::integral_constant<glm::length_t, 0> {};

template <typename T, glm::precision P>
struct glm_components<glm::tvec2<T, P>> : public std::integral_constant<
    glm::length_t, 2
> {};

template <typename T, glm::precision P>
struct glm_components<glm::tvec3<T, P>> : public std::integral_constant<
    glm::length_t, 3
> {};

template <typename T, glm::precision P>
struct glm_components<glm::tvec4<T, P>> : public std::integral_constant<
    glm::length_t, 4
> {};


template <typename T>
struct glm_rows : public std::integral_constant<glm::length_t, 0> {};

template <typename T, glm::precision P>
struct glm_rows<glm::tmat2x2<T, P>> : public std::integral_constant<glm::length_t, 2> {};

template <typename T, glm::precision P>
struct glm_rows<glm::tmat2x3<T, P>> : public std::integral_constant<glm::length_t, 3> {};

template <typename T, glm::precision P>
struct glm_rows<glm::tmat2x4<T, P>> : public std::integral_constant<glm::length_t, 4> {};

template <typename T, glm::precision P>
struct glm_rows<glm::tmat3x2<T, P>> : public std::integral_constant<glm::length_t, 2> {};

template <typename T, glm::precision P>
struct glm_rows<glm::tmat3x3<T, P>> : public std::integral_constant<glm::length_t, 3> {};

template <typename T, glm::precision P>
struct glm_rows<glm::tmat3x4<T, P>> : public std::integral_constant<glm::length_t, 4> {};

template <typename T, glm::precision P>
struct glm_rows<glm::tmat4x2<T, P>> : public std::integral_constant<glm::length_t, 2> {};

template <typename T, glm::precision P>
struct glm_rows<glm::tmat4x3<T, P>> : public std::integral_constant<glm::length_t, 3> {};

template <typename T, glm::precision P>
struct glm_rows<glm::tmat4x4<T, P>> : public std::integral_constant<glm::length_t, 4> {};


template <typename T>
struct glm_cols : public std::integral_constant<std::size_t, 0> {};

template <typename T, glm::precision P>
struct glm_cols<glm::tmat2x2<T, P>> : public std::integral_constant<glm::length_t, 2> {};

template <typename T, glm::precision P>
struct glm_cols<glm::tmat2x3<T, P>> : public std::integral_constant<glm::length_t, 2> {};

template <typename T, glm::precision P>
struct glm_cols<glm::tmat2x4<T, P>> : public std::integral_constant<glm::length_t, 2> {};

template <typename T, glm::precision P>
struct glm_cols<glm::tmat3x2<T, P>> : public std::integral_constant<glm::length_t, 3> {};

template <typename T, glm::precision P>
struct glm_cols<glm::tmat3x3<T, P>> : public std::integral_constant<glm::length_t, 3> {};

template <typename T, glm::precision P>
struct glm_cols<glm::tmat3x4<T, P>> : public std::integral_constant<glm::length_t, 3> {};

template <typename T, glm::precision P>
struct glm_cols<glm::tmat4x2<T, P>> : public std::integral_constant<glm::length_t, 4> {};

template <typename T, glm::precision P>
struct glm_cols<glm::tmat4x3<T, P>> : public std::integral_constant<glm::length_t, 4> {};

template <typename T, glm::precision P>
struct glm_cols<glm::tmat4x4<T, P>> : public std::integral_constant<glm::length_t, 4> {};

} // ghoul

namespace std {

inline std::string to_string(const glm::bvec2& _Val) {
    return "{" + std::to_string(_Val.x) + "," + std::to_string(_Val.y) + "}";
}

inline std::string to_string(const glm::bvec3& _Val) {
    return "{" +
        std::to_string(_Val.x) + "," +
        std::to_string(_Val.y) + "," +
        std::to_string(_Val.z) + "}";
}

inline std::string to_string(const glm::bvec4& _Val) {
    return "{" +
        std::to_string(_Val.x) + "," +
        std::to_string(_Val.y) + "," +
        std::to_string(_Val.z) + "," +
        std::to_string(_Val.w) + "}";
}

inline std::string to_string(const glm::vec2& _Val) {
    return "{" + std::to_string(_Val.x) + "," + std::to_string(_Val.y) + "}";
}

inline std::string to_string(const glm::vec3& _Val) {
    return "{" +
        std::to_string(_Val.x) + "," +
        std::to_string(_Val.y) + "," +
        std::to_string(_Val.z) + "}";
}

inline std::string to_string(const glm::vec4& _Val) {
    return "{" +
        std::to_string(_Val.x) + "," +
        std::to_string(_Val.y) + "," +
        std::to_string(_Val.z) + "," +
        std::to_string(_Val.w) + "}";
}

inline std::string to_string(const glm::dvec2& _Val) {
    return "{" + std::to_string(_Val.x) + "," + std::to_string(_Val.y) + "}";
}

inline std::string to_string(const glm::dvec3& _Val) {
    return "{" +
        std::to_string(_Val.x) + "," +
        std::to_string(_Val.y) + "," +
        std::to_string(_Val.z) + "}";
}

inline std::string to_string(const glm::dvec4& _Val) {
    return "{" +
        std::to_string(_Val.x) + "," +
        std::to_string(_Val.y) + "," +
        std::to_string(_Val.z) + "," +
        std::to_string(_Val.w) + "}";
}

inline std::string to_string(const glm::ivec2& _Val) {
    return "{" + std::to_string(_Val.x) + "," + std::to_string(_Val.y) + "}";
}

inline std::string to_string(const glm::ivec3& _Val) {
    return "{" +
        std::to_string(_Val.x) + "," +
        std::to_string(_Val.y) + "," +
        std::to_string(_Val.z) + "}";
}

inline std::string to_string(const glm::ivec4& _Val) {
    return "{" +
        std::to_string(_Val.x) + "," +
        std::to_string(_Val.y) + "," +
        std::to_string(_Val.z) + "," +
        std::to_string(_Val.w) + "}";
}

inline std::string to_string(const glm::uvec2& _Val) {
    return "{" + std::to_string(_Val.x) + "," + std::to_string(_Val.y) + "}";
}

inline std::string to_string(const glm::uvec3& _Val) {
    return "{" +
        std::to_string(_Val.x) + "," +
        std::to_string(_Val.y) + "," +
        std::to_string(_Val.z) + "}";
}

inline std::string to_string(const glm::uvec4& _Val) {
    return "{" +
        std::to_string(_Val.x) + "," +
        std::to_string(_Val.y) + "," +
        std::to_string(_Val.z) + "," +
        std::to_string(_Val.w) + "}";
}

inline std::string to_string(const glm::mat2& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "}";
}

inline std::string to_string(const glm::mat2x3& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "}";
}

inline std::string to_string(const glm::mat2x4& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[0].w) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "," +
        std::to_string(_Val[1].w) + "}";
}

inline std::string to_string(const glm::mat3x2& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "}";
}

inline std::string to_string(const glm::mat3& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "," +
        std::to_string(_Val[2].z) + "}";
}

inline std::string to_string(const glm::mat3x4& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[0].w) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "," +
        std::to_string(_Val[1].w) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "," +
        std::to_string(_Val[2].z) + "," +
        std::to_string(_Val[2].w) + "}";
}

inline std::string to_string(const glm::mat4x2& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "," +
        std::to_string(_Val[3].x) + "," +
        std::to_string(_Val[3].y) + "}";
}

inline std::string to_string(const glm::mat4x3& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "," +
        std::to_string(_Val[2].z) + "," +
        std::to_string(_Val[3].x) + "," +
        std::to_string(_Val[3].y) + "," +
        std::to_string(_Val[3].z) + "}";
}

inline std::string to_string(const glm::mat4& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[0].w) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "," +
        std::to_string(_Val[1].w) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "," +
        std::to_string(_Val[2].z) + "," +
        std::to_string(_Val[2].w) + "," +
        std::to_string(_Val[3].x) + "," +
        std::to_string(_Val[3].y) + "," +
        std::to_string(_Val[3].z) + "," +
        std::to_string(_Val[3].w) + "}";
}

inline std::string to_string(const glm::dmat2& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "}";
}

inline std::string to_string(const glm::dmat2x3& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "}";
}

inline std::string to_string(const glm::dmat2x4& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[0].w) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "," +
        std::to_string(_Val[1].w) + "}";
}

inline std::string to_string(const glm::dmat3x2& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "}";
}

inline std::string to_string(const glm::dmat3& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "," +
        std::to_string(_Val[2].z) + "}";
}

inline std::string to_string(const glm::dmat3x4& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[0].w) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "," +
        std::to_string(_Val[1].w) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "," +
        std::to_string(_Val[2].z) + "," +
        std::to_string(_Val[2].w) + "}";
}

inline std::string to_string(const glm::dmat4x2& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "," +
        std::to_string(_Val[3].x) + "," +
        std::to_string(_Val[3].y) + "}";
}

inline std::string to_string(const glm::dmat4x3& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "," +
        std::to_string(_Val[2].z) + "," +
        std::to_string(_Val[3].x) + "," +
        std::to_string(_Val[3].y) + "," +
        std::to_string(_Val[3].z) + "}";
}

inline std::string to_string(const glm::dmat4& _Val) {
    return "{" +
        std::to_string(_Val[0].x) + "," +
        std::to_string(_Val[0].y) + "," +
        std::to_string(_Val[0].z) + "," +
        std::to_string(_Val[0].w) + "," +
        std::to_string(_Val[1].x) + "," +
        std::to_string(_Val[1].y) + "," +
        std::to_string(_Val[1].z) + "," +
        std::to_string(_Val[1].w) + "," +
        std::to_string(_Val[2].x) + "," +
        std::to_string(_Val[2].y) + "," +
        std::to_string(_Val[2].z) + "," +
        std::to_string(_Val[2].w) + "," +
        std::to_string(_Val[3].x) + "," +
        std::to_string(_Val[3].y) + "," +
        std::to_string(_Val[3].z) + "," +
        std::to_string(_Val[3].w) + "}";
}

} // namespace std

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tvec2<T,P>& val) {
    return out << std::to_string(val);
}

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tvec3<T,P>& val) {
    return out << std::to_string(val);
}

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tvec4<T,P>& val) {
    return out << std::to_string(val);
}

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tmat2x2<T,P>& val) {
    return out << std::to_string(val);
}

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tmat2x3<T,P>& val) {
    return out << std::to_string(val);
}

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tmat2x4<T,P>& val) {
    return out << std::to_string(val);
}

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tmat3x2<T,P>& val) {
    return out << std::to_string(val);
}

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tmat3x3<T,P>& val) {
    return out << std::to_string(val);
}

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tmat3x4<T,P>& val) {
    return out << std::to_string(val);
}

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tmat4x2<T,P>& val) {
    return out << std::to_string(val);
}

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tmat4x3<T,P>& val) {
    return out << std::to_string(val);
}

template <typename T, glm::precision P>
std::ostream& operator<<(std::ostream& out, const glm::tmat4x4<T,P>& val) {
    return out << std::to_string(val);
}


#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

#ifdef __unix__
#pragma GCC diagnostic pop
#endif // __unix__

#endif // __GHOUL___GLM___H__
