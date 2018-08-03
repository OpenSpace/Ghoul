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

#ifndef __GHOUL___UNIFORMCACHE___H__
#define __GHOUL___UNIFORMCACHE___H__

// Dear future-me (abock)
// I gave up trying to make this prettier for a reason
// I tried moving the number of parameters into a parameter:
// #define InternalUniformCache_1(__x1__) struct { int __x1__ = -1 }
// #define InternalUniformCache_2(__x1__, __x2__) struct { int __x1__ = -1, __x2__ = -1 }
// #define UniformCache(n, ...) InternalUniformCache_##n(__VA_ARGS__)
// But that leads to struct { int __x1__, __x2__ = =1, = -1 } due to the way the parameter
// expansion works

// I tried making the function recursive as by
// https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms
// but that code failed to work in Visual Studio

// I tried creating a new struct that autoinitializes to get rid of the = -1 at the end
// but that failed as it needs to be cast into an int& for the caching

// This solution is from here:
// https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
// The mutable was added to the struct to make them usable in `const` methods


// HOW TO ADD MORE PARAMETERS:
// 1. Add a new macro UniformCacheX, copying from X-1
// 2. Add vX variable to argument list
// 3. Add ,vX = -1 to the struct definition
// 4. Add _X before NAME in the GET_MACRO macro
// 5. Add UniformCacheX after __VA_ARGS__ in the UniformCache macro

// OBS: Refrain from adding new values to the struct, or at the very least, update the
//      updateUniformLocations method as it depends on the UniformCache's struct size

#include <ghoul/opengl/programobject.h>
#include <array>

/// This is just an empty tag that can be used to provide better error messages for the
/// updateUniformLocations method
struct UniformCacheBase {};

#define UniformCache1(v1)                                                                \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1;                                                                     \
    }

#define UniformCache2(v1, v2)                                                            \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1, v2 = -1;                                                            \
    }

#define UniformCache3(v1, v2, v3)                                                        \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1, v2 = -1, v3 = -1;                                                   \
    }

#define UniformCache4(v1, v2, v3, v4)                                                    \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1, v2 = -1, v3 = -1, v4 = -1;                                          \
    }

#define UniformCache5(v1, v2, v3, v4, v5)                                                \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1, v2 = -1, v3 = -1, v4 = -1, v5 = -1;                                 \
}

#define UniformCache6(v1, v2, v3, v4, v5, v6)                                            \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1, v2 = -1, v3 = -1, v4 = -1, v5 = -1, v6 = -1;                        \
    }

#define UniformCache7(v1, v2, v3, v4, v5, v6, v7)                                        \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1, v2 = -1, v3 = -1, v4 = -1, v5 = -1, v6 = -1, v7 = -1;               \
    }

#define UniformCache8(v1, v2, v3, v4, v5, v6, v7, v8)                                    \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1, v2 = -1, v3 = -1, v4 = -1, v5 = -1, v6 = -1, v7 = -1, v8 = -1;      \
    }

#define UniformCache9(v1, v2, v3, v4, v5, v6, v7, v8, v9)                                \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1, v2 = -1, v3 = -1, v4 = -1, v5 = -1, v6 = -1, v7 = -1, v8 = -1,      \
            v9 = -1;                                                                     \
    }

#define UniformCache10(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10)                          \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1,  v2 = -1, v3 = -1, v4 = -1, v5 = -1, v6 = -1, v7 = -1, v8 = -1,     \
            v9 = -1, v10 = -1;                                                           \
    }

#define UniformCache11(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11)                     \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1,  v2 = -1,  v3 = -1, v4 = -1, v5 = -1, v6 = -1, v7 = -1, v8 = -1,    \
            v9 = -1, v10 = -1, v11 = -1;                                                 \
    }

#define UniformCache12(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12)                \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1, v5 = -1, v6 = -1, v7 = -1, v8 = -1,   \
            v9 = -1, v10 = -1, v11 = -1, v12 = -1;                                       \
    }

#define UniformCache13(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13)           \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1, v6 = -1, v7 = -1, v8 = -1,  \
            v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1;                             \
    }

#define UniformCache14(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14)      \
    struct : public UniformCacheBase {                                                   \
        int v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1, v7 = -1, v8 = -1, \
            v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1;                   \
    }

#define UniformCache15(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15) \
    struct : public UniformCacheBase {                                                   \
        int  v1 = -1, v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1,  v7 = -1,         \
             v8 = -1, v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1,         \
            v15 = -1;                                                                    \
    }

#define UniformCache16(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                       v16)                                                              \
    struct : public UniformCacheBase {                                                   \
        int  v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1,  v7 = -1,        \
             v8 = -1,  v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1,        \
            v15 = -1, v16 = -1;                                                          \
    }

#define UniformCache17(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                       v16, v17)                                                         \
    struct : public UniformCacheBase {                                                   \
        int  v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1,  v7 = -1,        \
             v8 = -1,  v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1,        \
            v15 = -1, v16 = -1, v17 = -1;                                                \
    }

#define UniformCache18(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                       v16, v17, v18)                                                    \
    struct : public UniformCacheBase {                                                   \
        int  v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1,  v7 = -1,        \
             v8 = -1,  v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1,        \
            v15 = -1, v16 = -1, v17 = -1, v18 = -1;                                      \
    }

#define UniformCache19(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                       v16, v17, v18, v19)                                               \
    struct : public UniformCacheBase {                                                   \
        int  v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1,  v7 = -1,        \
             v8 = -1,  v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1,        \
            v15 = -1, v16 = -1, v17 = -1, v18 = -1, v19 = -1;                            \
    }

#define UniformCache20(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                       v16, v17, v18, v19, v20)                                          \
    struct : public UniformCacheBase {                                                   \
        int  v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1,  v7 = -1,        \
             v8 = -1,  v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1,        \
            v15 = -1, v16 = -1, v17 = -1, v18 = -1, v19 = -1, v20 = -1;                  \
    }

#define UniformCache21(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                       v16, v17, v18, v19, v20, v21)                                     \
    struct : public UniformCacheBase {                                                   \
        int  v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1,  v7 = -1,        \
             v8 = -1,  v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1,        \
            v15 = -1, v16 = -1, v17 = -1, v18 = -1, v19 = -1, v20 = -1, v21 = -1;        \
    }

#define UniformCache22(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                       v16, v17, v18, v19, v20, v21, v22)                                \
    struct : public UniformCacheBase {                                                   \
        int  v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1,  v7 = -1,        \
             v8 = -1,  v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1,        \
            v15 = -1, v16 = -1, v17 = -1, v18 = -1, v19 = -1, v20 = -1, v21 = -1,        \
            v22 = -1;                                                                    \
    }

#define UniformCache23(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                       v16, v17, v18, v19, v20, v21, v22, v23)                           \
    struct : public UniformCacheBase {                                                   \
        int  v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1,  v7 = -1,        \
             v8 = -1,  v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1,        \
            v15 = -1, v16 = -1, v17 = -1, v18 = -1, v19 = -1, v20 = -1, v21 = -1,        \
            v22 = -1, v23 = -1;                                                          \
    }

#define UniformCache24(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                       v16, v17, v18, v19, v20, v21, v22, v23, v24)                      \
    struct : public UniformCacheBase {                                                   \
        int  v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1,  v7 = -1,        \
             v8 = -1,  v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1,        \
            v15 = -1, v16 = -1, v17 = -1, v18 = -1, v19 = -1, v20 = -1, v21 = -1,        \
            v22 = -1, v23 = -1, v24 = -1;                                                \
    }

#define UniformCache25(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                       v16, v17, v18, v19, v20, v21, v22, v23, v24, v25)                 \
    struct : public UniformCacheBase {                                                   \
        int  v1 = -1,  v2 = -1,  v3 = -1,  v4 = -1,  v5 = -1,  v6 = -1,  v7 = -1,        \
             v8 = -1,  v9 = -1, v10 = -1, v11 = -1, v12 = -1, v13 = -1, v14 = -1,        \
            v15 = -1, v16 = -1, v17 = -1, v18 = -1, v19 = -1, v20 = -1, v21 = -1,        \
            v22 = -1, v23 = -1, v24 = -1, v25 = -1;                                      \
    }

namespace ghoul::opengl {

/**
 * Function that will update the uniform locations of the \p uniformCache based on
 * querying the \p uniformNames in the \p program. This method will essentially call
 * ProgramObject::uniformLocation on the passed parameters. The order of arguments in the
 * \p uniformNames has to be the same as the location definitions.
 *
 * \tparam
 */
template <typename T> // @CONCEPTS
void updateUniformLocations(const ghoul::opengl::ProgramObject& program, T& uniformCache,
                     const std::array<const char*, sizeof(T) / sizeof(int)>& uniformNames)
{
    // The UniformCaches must only contain data for the uniform locations, so they must be
    // divisible by 4 as each is an integer number
    static_assert(sizeof(T) % sizeof(int) == 0);

    // Make the error message a bit nicer if the 
    static_assert(std::is_base_of_v<UniformCacheBase, T>);

    constexpr const int nValues = sizeof(T) / sizeof(int);

    int* uniformLocations = reinterpret_cast<int*>(&uniformCache);
    for (int i = 0; i < nValues; ++i) {
        // We skip empty uniform names
        if (uniformNames[i]) {
            uniformLocations[i] = program.uniformLocation(uniformNames[i]);
        }
    }
}

} // namespace ghoul::opengl


// The EXPAND macro is a fix for MSVC's interpretation of the C++11 standard of VA_ARGS
#define EXPAND(x) x
#define GET_MACRO( _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, _10, _11, _12, _13, _14,  \
                  _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, NAME, ...) NAME

#define UniformCache(...)                                                                \
    EXPAND(                                                                              \
        GET_MACRO(                                                                       \
            __VA_ARGS__,                                                                 \
            UniformCache25, UniformCache24, UniformCache23, UniformCache22,              \
            UniformCache21, UniformCache20, UniformCache19, UniformCache18,              \
            UniformCache17, UniformCache16, UniformCache15, UniformCache14,              \
            UniformCache13, UniformCache12, UniformCache11, UniformCache10,              \
             UniformCache9,  UniformCache8,  UniformCache7,  UniformCache6,              \
             UniformCache5,  UniformCache4,  UniformCache3,  UniformCache2,              \
             UniformCache1                                                               \
        )(__VA_ARGS__)                                                                   \
    )

#endif // __GHOUL___UNIFORMCACHE___H__
