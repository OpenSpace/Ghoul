/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2019                                                               *
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

#include "catch2/catch.hpp"

#include <ghoul/misc/dictionary.h>
#include <ghoul/glm.h>
#include <fstream>
#include <sstream>

 /*
  * Test checklist:
  * +++ getValue
  * +++  basic types
  * +++  advanced types
  * +++ setValues
  * +++  basic types
  * +++  advanced types
  * +++ nested dictionaries
  * +++ timing
  */

namespace {
    ghoul::Dictionary createDefaultDictionary() {
        return {
            { "bool", true },
            { "char", char(1) },
            { "signed char", static_cast<signed char>(1) },
            { "unsigned char", static_cast<unsigned char>(1) },
            { "wchar_t", wchar_t(1) },
            { "short", short(1) },
            { "unsigned short", static_cast<unsigned short>(1) },
            { "int", int(1) },
            { "unsigned int", static_cast<unsigned int>(1) },
            { "long long", static_cast<long long>(1) },
            { "unsigned long long", static_cast<unsigned long long>(1) },
            { "float", float(1) },
            { "double", double(1) },
            { "long double", static_cast<long double>(1) },
            { "dictionary", ghoul::Dictionary() },
            { "vec2", glm::vec2(1.f, 2.f) },
            { "dvec2", glm::dvec2(1.0, 2.0) },
            { "ivec2", glm::ivec2(1, 2) },
            { "uvec2", glm::uvec2(1, 2) },
            { "bvec2", glm::bvec2(true, false) },
            { "vec3", glm::vec3(1.f, 2.f, 3.f) },
            { "dvec3", glm::dvec3(1.0, 2.0, 3.0) },
            { "ivec3", glm::ivec3(1, 2, 3) },
            { "uvec3", glm::uvec3(1, 2, 3) },
            { "bvec3", glm::bvec3(true, false, true) },
            { "vec4", glm::vec4(1.f, 2.f, 3.f, 4.f) },
            { "dvec4", glm::dvec4(1.0, 2.0, 3.0, 4.0) },
            { "ivec4", glm::ivec4(1, 2, 3, 4) },
            { "uvec4", glm::uvec4(1, 2, 3, 4) },
            { "bvec4", glm::bvec4(true, false, true, false) },
            { "mat2x2", glm::mat2x2(1.f, 2.f, 3.f, 4.f) },
            { "mat2x3", glm::mat2x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f) },
            { "mat2x4", glm::mat2x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f) },
            { "mat3x2", glm::mat3x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f) },
            { "mat3x3", glm::mat3x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f) },
            { "mat3x4", glm::mat3x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f,
                                   11.f, 12.f) },
            { "mat4x2", glm::mat4x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f) },
            { "mat4x3", glm::mat4x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f,
                                   11.f, 12.f) },
            { "mat4x4", glm::mat4x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f,
                                   11.f, 12.f, 13.f, 14.f, 15.f, 16.f) },
            { "dmat2x2", glm::dmat2x2(1.0, 2.0, 3.0, 4.0) },
            { "dmat2x3", glm::dmat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0) },
            { "dmat2x4", glm::dmat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0) },
            { "dmat3x2", glm::dmat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0) },
            { "dmat3x3", glm::dmat3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0) },
            { "dmat3x4", glm::dmat3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                                     11.0, 12.0) },
            { "dmat4x2", glm::dmat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0) },
            { "dmat4x3", glm::dmat4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                                     11.0, 12.0) },
            { "dmat4x4", glm::dmat4x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                                     11.0, 12.0, 13.0, 14.0, 15.0, 16.0) }
        };
    }

} // namespace

TEST_CASE("Dictionary: Empty", "[dictionary]") {
    ghoul::Dictionary d;
    REQUIRE(d.size() == 0);

}

TEST_CASE("Dictionary: Clear", "[dictionary]") {
    ghoul::Dictionary d;
    
    d.setValue("a", 1);
    REQUIRE(d.size() == 1);
    d.clear();
    REQUIRE(d.size() == 0);
}

TEST_CASE("Dictionary: Initializer Lists", "[dictionary]") {
    ghoul::Dictionary d = { { "a", 1 } };
    ghoul::Dictionary d2 = { { "a", 1 }, { "b", 2 } };
    ghoul::Dictionary d3 = { { "a", 1 }, { "b", 2 }, { "c", 3 } };

    REQUIRE(d.size() == 1);
    REQUIRE(d2.size() == 2);
    REQUIRE(d3.size() == 3);
}

TEST_CASE("Dictionary: Nested Dictionaries", "[dictionary]") {
    ghoul::Dictionary d = { { "a", 1 } };
    ghoul::Dictionary e = { { "a", 1 }, { "b", d } };

    REQUIRE(e.size() == 2);
}

TEST_CASE("Dictionary: Assignment Operator", "[dictionary]") {
    ghoul::Dictionary d = { { "a", 1 }, { "b", 2 } };
    ghoul::Dictionary e = d;
    REQUIRE(e.size() == 2);
    int value;
    const bool successA = d.getValue("a", value);
    REQUIRE(successA);
    REQUIRE(value == 1);
    const bool successB = d.getValue("b", value);
    REQUIRE(successB);
    REQUIRE(value == 2);
}

TEST_CASE("Dictionary: Copy Constructor", "[dictionary]") {
    ghoul::Dictionary d = { { "a", 1 }, { "b", 2 } };
    ghoul::Dictionary e(d);
    REQUIRE(e.size() == 2);
    int value;
    const bool successA = d.getValue("a", value);
    REQUIRE(successA);
    REQUIRE(value == 1);
    const bool successB = d.getValue("b", value);
    REQUIRE(successB);
    REQUIRE(value == 2);
}

TEST_CASE("Dictionary: Has Value", "[dictionary]") {
    ghoul::Dictionary d = createDefaultDictionary();

    // correct types
    REQUIRE(d.size() == 48);
    REQUIRE(d.hasValue<bool>("bool"));
    REQUIRE(d.hasValue<char>("char"));
    REQUIRE(d.hasValue<signed char>("signed char"));
    REQUIRE(d.hasValue<unsigned char>("unsigned char"));
    REQUIRE(d.hasValue<wchar_t>("wchar_t"));
    REQUIRE(d.hasValue<short>("short"));
    REQUIRE(d.hasValue<unsigned short>("unsigned short"));
    REQUIRE(d.hasValue<int>("int"));
    REQUIRE(d.hasValue<unsigned int>("unsigned int"));
    REQUIRE(d.hasValue<long long>("long long"));
    REQUIRE(d.hasValue<unsigned long long>("unsigned long long"));
    REQUIRE(d.hasValue<float>("float"));
    REQUIRE(d.hasValue<double>("double"));
    REQUIRE(d.hasValue<long double>("long double"));
    REQUIRE(d.hasValue<ghoul::Dictionary>("dictionary"));
    REQUIRE(d.hasValue<glm::vec2>("vec2"));
    REQUIRE(d.hasValue<glm::dvec2>("dvec2"));
    REQUIRE(d.hasValue<glm::ivec2>("ivec2"));
    REQUIRE(d.hasValue<glm::uvec2>("uvec2"));
    REQUIRE(d.hasValue<glm::bvec2>("bvec2"));
    REQUIRE(d.hasValue<glm::vec3>("vec3"));
    REQUIRE(d.hasValue<glm::dvec3>("dvec3"));
    REQUIRE(d.hasValue<glm::ivec3>("ivec3"));
    REQUIRE(d.hasValue<glm::uvec3>("uvec3"));
    REQUIRE(d.hasValue<glm::bvec3>("bvec3"));
    REQUIRE(d.hasValue<glm::vec4>("vec4"));
    REQUIRE(d.hasValue<glm::dvec4>("dvec4"));
    REQUIRE(d.hasValue<glm::ivec4>("ivec4"));
    REQUIRE(d.hasValue<glm::uvec4>("uvec4"));
    REQUIRE(d.hasValue<glm::bvec4>("bvec4"));
    REQUIRE(d.hasValue<glm::mat2x2>("mat2x2"));
    REQUIRE(d.hasValue<glm::mat2x3>("mat2x3"));
    REQUIRE(d.hasValue<glm::mat2x4>("mat2x4"));
    REQUIRE(d.hasValue<glm::mat3x2>("mat3x2"));
    REQUIRE(d.hasValue<glm::mat3x3>("mat3x3"));
    REQUIRE(d.hasValue<glm::mat3x4>("mat3x4"));
    REQUIRE(d.hasValue<glm::mat4x2>("mat4x2"));
    REQUIRE(d.hasValue<glm::mat4x3>("mat4x3"));
    REQUIRE(d.hasValue<glm::mat4x4>("mat4x4"));
    REQUIRE(d.hasValue<glm::dmat2x2>("dmat2x2"));
    REQUIRE(d.hasValue<glm::dmat2x3>("dmat2x3"));
    REQUIRE(d.hasValue<glm::dmat2x4>("dmat2x4"));
    REQUIRE(d.hasValue<glm::dmat3x2>("dmat3x2"));
    REQUIRE(d.hasValue<glm::dmat3x3>("dmat3x3"));
    REQUIRE(d.hasValue<glm::dmat3x4>("dmat3x4"));
    REQUIRE(d.hasValue<glm::dmat4x2>("dmat4x2"));
    REQUIRE(d.hasValue<glm::dmat4x3>("dmat4x3"));
    REQUIRE(d.hasValue<glm::dmat4x4>("dmat4x4"));

    // false types
    REQUIRE_FALSE(d.hasValue<int>("float"));

    // not existing
    REQUIRE_FALSE(d.hasValue<bool>("nokey"));
}

TEST_CASE("Dictionary: Get Value", "[dictionary]") {
    ghoul::Dictionary d = createDefaultDictionary();
    REQUIRE(d.size() == 48);
    {
        bool value;
        const bool success = d.getValue("bool", value);
        REQUIRE(success);
        REQUIRE(value);
    }
    {
        char value;
        const bool success = d.getValue("char", value);
        REQUIRE(success);
        REQUIRE(value == char(1));
    }
    {
        signed char value;
        const bool success = d.getValue("signed char", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<signed char>(1));
    }
    {
        unsigned char value;
        const bool success = d.getValue("unsigned char", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned char>(1));
    }
    {
        wchar_t value;
        const bool success = d.getValue("wchar_t", value);
        REQUIRE(success);
        REQUIRE(value == wchar_t(1));
    }
    {
        short value;
        const bool success = d.getValue("short", value);
        REQUIRE(success);
        REQUIRE(value == short(1));
    }
    {
        unsigned short value;
        const bool success = d.getValue("unsigned short", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned short>(1));
    }
    {
        int value;
        const bool success = d.getValue("int", value);
        REQUIRE(success);
        REQUIRE(value == int(1));
    }
    {
        unsigned int value;
        const bool success = d.getValue("unsigned int", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned int>(1));
    }
    {
        long long value;
        const bool success = d.getValue("long long", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<long long>(1));
    }
    {
        unsigned long long value;
        const bool success = d.getValue("unsigned long long", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned long long>(1));
    }
    {
        float value;
        const bool success = d.getValue("float", value);
        REQUIRE(success);
        REQUIRE(value == float(1));
    }
    {
        double value;
        const bool success = d.getValue("double", value);
        REQUIRE(success);
        REQUIRE(value == double(1));
    }
    {
        long double value;
        const bool success = d.getValue("long double", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<long double>(1));
    }
    {
        glm::vec2 value;
        const bool success = d.getValue("vec2", value);
        REQUIRE(success);
        REQUIRE(value == glm::vec2(1.f, 2.f));
    }
    {
        glm::dvec2 value;
        const bool success = d.getValue("dvec2", value);
        REQUIRE(success);
        REQUIRE(value == glm::dvec2(1.0, 2.0));
    }
    {
        glm::ivec2 value;
        const bool success = d.getValue("ivec2", value);
        REQUIRE(success);
        REQUIRE(value == glm::ivec2(1, 2));
    }
    {
        glm::uvec2 value;
        const bool success = d.getValue("uvec2", value);
        REQUIRE(success);
        REQUIRE(value == glm::uvec2(1, 2));
    }
    {
        glm::bvec2 value;
        const bool success = d.getValue("bvec2", value);
        REQUIRE(success);
        REQUIRE(value == glm::bvec2(true, false));
    }
    {
        glm::vec3 value;
        const bool success = d.getValue("vec3", value);
        REQUIRE(success);
        REQUIRE(value == glm::vec3(1.f, 2.f, 3.f));
    }
    {
        glm::dvec3 value;
        const bool success = d.getValue("dvec3", value);
        REQUIRE(success);
        REQUIRE(value == glm::dvec3(1.0, 2.0, 3.0));
    }
    {
        glm::ivec3 value;
        const bool success = d.getValue("ivec3", value);
        REQUIRE(success);
        REQUIRE(value == glm::ivec3(1, 2, 3));
    }
    {
        glm::uvec3 value;
        const bool success = d.getValue("uvec3", value);
        REQUIRE(success);
        REQUIRE(value == glm::uvec3(1, 2, 3));
    }
    {
        glm::bvec3 value;
        const bool success = d.getValue("bvec3", value);
        REQUIRE(success);
        REQUIRE(value == glm::bvec3(true, false, true));
    }
    {
        glm::vec4 value;
        const bool success = d.getValue("vec4", value);
        REQUIRE(success);
        REQUIRE(value == glm::vec4(1.f, 2.f, 3.f, 4.f));
    }
    {
        glm::dvec4 value;
        const bool success = d.getValue("dvec4", value);
        REQUIRE(success);
        REQUIRE(value == glm::dvec4(1.0, 2.0, 3.0, 4.0));
    }
    {
        glm::ivec4 value;
        const bool success = d.getValue("ivec4", value);
        REQUIRE(success);
        REQUIRE(value == glm::ivec4(1, 2, 3, 4));
    }
    {
        glm::uvec4 value;
        const bool success = d.getValue("uvec4", value);
        REQUIRE(success);
        REQUIRE(value == glm::uvec4(1, 2, 3, 4));
    }
    {
        glm::bvec4 value;
        const bool success = d.getValue("bvec4", value);
        REQUIRE(success);
        REQUIRE(value == glm::bvec4(true, false, true, false));
    }
    {
        glm::mat2x2 value;
        const bool success = d.getValue("mat2x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat2x2(1.f, 2.f, 3.f, 4.f));
    }
    {
        glm::mat2x3 value;
        const bool success = d.getValue("mat2x3", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat2x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f));
    }
    {
        glm::mat2x4 value;
        const bool success = d.getValue("mat2x4", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat2x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f));
    }
    {
        glm::mat3x2 value;
        const bool success = d.getValue("mat3x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat3x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f));
    }
    {
        glm::mat3x3 value;
        const bool success = d.getValue("mat3x3", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat3x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f));
    }
    {
        glm::mat3x4 value;
        const bool success = d.getValue("mat3x4", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            glm::mat3x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f)
        );
    }
    {
        glm::mat4x2 value;
        const bool success = d.getValue("mat4x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat4x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f));
    }
    {
        glm::mat4x3 value;
        const bool success = d.getValue("mat4x3", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            glm::mat4x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f)
        );
    }
    {
        glm::mat4x4 value;
        const bool success = d.getValue("mat4x4", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            glm::mat4x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f,
                12.f, 13.f, 14.f, 15.f, 16.f)
        );
    }
    {
        glm::dmat2x2 value;
        const bool success = d.getValue("dmat2x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat2x2(1.0, 2.0, 3.0, 4.0));
    }
    {
        glm::dmat2x3 value;
        const bool success = d.getValue("dmat2x3", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0));
    }
    {
        glm::dmat2x4 value;
        const bool success = d.getValue("dmat2x4", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0));
    }
    {
        glm::dmat3x2 value;
        const bool success = d.getValue("dmat3x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0));
    }
    {
        glm::dmat3x3 value;
        const bool success = d.getValue("dmat3x3", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0));
    }
    {
        glm::dmat3x4 value;
        const bool success = d.getValue("dmat3x4", value);
        REQUIRE(success);
        REQUIRE(             
            value ==
            glm::dmat3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0)
        );
    }
    {
        glm::dmat4x2 value;
        const bool success = d.getValue("dmat4x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0));
    }
    {
        glm::dmat4x3 value;
        const bool success = d.getValue("dmat4x3", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            glm::dmat4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0)
        );
    }
    {
        glm::dmat4x4 value;
        const bool success = d.getValue("dmat4x4", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            glm::dmat4x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
                12.0, 13.0, 14.0, 15.0, 16.0)
        );
    }
}

TEST_CASE("Dictionary: Set Value", "[dictionary]") {
    ghoul::Dictionary d;
    SECTION("bool") {
        REQUIRE_FALSE(d.hasValue<bool>("bool"));
        d.setValue("bool", bool(true));
        REQUIRE(d.hasValue<bool>("bool"));
        bool value;
        const bool success = d.getValue("bool", value);
        REQUIRE(success);
        REQUIRE(value);
    }
    SECTION("char") {
        REQUIRE_FALSE(d.hasValue<char>("char"));
        d.setValue("char", char(1));
        REQUIRE(d.hasValue<char>("char"));
        char value;
        const bool success = d.getValue("char", value);
        REQUIRE(success);
        REQUIRE(value == char(1));
    }
    SECTION("signed char") {
        REQUIRE_FALSE(d.hasValue<signed char>("signed char"));
        d.setValue("signed char", static_cast<signed char>(1));
        REQUIRE(d.hasValue<signed char>("signed char"));
        signed char value;
        const bool success = d.getValue("signed char", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<signed char>(1));
    }
    SECTION("unsigned char") {
        REQUIRE_FALSE(d.hasValue<unsigned char>("unsigned char"));
        d.setValue("unsigned char", static_cast<unsigned char>(1));
        REQUIRE(d.hasValue<unsigned char>("unsigned char"));
        unsigned char value;
        const bool success = d.getValue("unsigned char", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned char>(1));
    }
    SECTION("wchar_t") {
        REQUIRE_FALSE(d.hasValue<wchar_t>("wchar_t"));
        d.setValue("wchar_t", wchar_t(1));
        REQUIRE(d.hasValue<wchar_t>("wchar_t"));
        wchar_t value;
        const bool success = d.getValue("wchar_t", value);
        REQUIRE(success);
        REQUIRE(value == wchar_t(1));
    }
    SECTION("short") {
        REQUIRE_FALSE(d.hasValue<short>("short"));
        d.setValue("short", short(1));
        REQUIRE(d.hasValue<short>("short"));
        short value;
        const bool success = d.getValue("short", value);
        REQUIRE(success);
        REQUIRE(value == short(1));
    }
    SECTION("unsigned short") {
        REQUIRE_FALSE(d.hasValue<unsigned short>("unsigned short"));
        d.setValue("unsigned short", static_cast<unsigned short>(1));
        REQUIRE(d.hasValue<unsigned short>("unsigned short"));
        unsigned short value;
        const bool success = d.getValue("unsigned short", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned short>(1));
    }
    SECTION("int") {
        REQUIRE_FALSE(d.hasValue<int>("int"));
        d.setValue("int", int(1));
        REQUIRE(d.hasValue<int>("int"));
        int value;
        const bool success = d.getValue("int", value);
        REQUIRE(success);
        REQUIRE(value == int(1));
    }
    SECTION("unsigned int") {
        REQUIRE_FALSE(d.hasValue<unsigned int>("unsigned int"));
        d.setValue("unsigned int", static_cast<unsigned int>(1));
        REQUIRE(d.hasValue<unsigned int>("unsigned int"));
        unsigned int value;
        const bool success = d.getValue("unsigned int", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned int>(1));
    }
    SECTION("long long") {
        REQUIRE_FALSE(d.hasValue<long long>("long long"));
        d.setValue("long long", static_cast<long long>(1));
        REQUIRE(d.hasValue<long long>("long long"));
        long long value;
        const bool success = d.getValue("long long", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<long long>(1));
    }
    SECTION("unsigned long long") {
        REQUIRE_FALSE(d.hasValue<unsigned long long>("unsigned long long"));
        d.setValue("unsigned long long", static_cast<unsigned long long>(1));
        REQUIRE(d.hasValue<unsigned long long>("unsigned long long"));
        unsigned long long value;
        const bool success = d.getValue("unsigned long long", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned long long>(1));
    }
    SECTION("float") {
        REQUIRE_FALSE(d.hasValue<float>("float"));
        d.setValue("float", 1.f);
        REQUIRE(d.hasValue<float>("float"));
        float value;
        const bool success = d.getValue("float", value);
        REQUIRE(success);
        REQUIRE(value == 1.f);
    }
    SECTION("double") {
        REQUIRE_FALSE(d.hasValue<double>("double"));
        d.setValue("double", 1.0);
        REQUIRE(d.hasValue<double>("double"));
        double value;
        const bool success = d.getValue("double", value);
        REQUIRE(success);
        REQUIRE(value == 1.0);
    }
    SECTION("long double") {
        REQUIRE_FALSE(d.hasValue<long double>("long double"));
        d.setValue("long double", static_cast<long double>(1));
        REQUIRE(d.hasValue<long double>("long double"));
        long double value;
        const bool success = d.getValue("long double", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<long double>(1));
    }
    SECTION("vec2") {
        REQUIRE_FALSE(d.hasValue<glm::vec2>("vec2"));
        d.setValue("vec2", glm::vec2(1.f, 2.f));
        REQUIRE(d.hasValue<glm::vec2>("vec2"));
        glm::vec2 value;
        const bool success = d.getValue("vec2", value);
        REQUIRE(success);
        REQUIRE(value == glm::vec2(1.f, 2.f));
    }
    SECTION("dvec2") {
        REQUIRE_FALSE(d.hasValue<glm::dvec2>("dvec2"));
        d.setValue("dvec2", glm::dvec2(1.0, 2.0));
        REQUIRE(d.hasValue<glm::dvec2>("dvec2"));
        glm::dvec2 value;
        const bool success = d.getValue("dvec2", value);
        REQUIRE(success);
        REQUIRE(value == glm::dvec2(1.0, 2.0));
    }
    SECTION("ivec2") {
        REQUIRE_FALSE(d.hasValue<glm::ivec2>("ivec2"));
        d.setValue("ivec2", glm::ivec2(1, 2));
        REQUIRE(d.hasValue<glm::ivec2>("ivec2"));
        glm::ivec2 value;
        const bool success = d.getValue("ivec2", value);
        REQUIRE(success);
        REQUIRE(value == glm::ivec2(1, 2));
    }
    SECTION("bvec2") {
        REQUIRE_FALSE(d.hasValue<glm::bvec2>("bvec2"));
        d.setValue("bvec2", glm::bvec2(true, false));
        REQUIRE(d.hasValue<glm::bvec2>("bvec2"));
        glm::bvec2 value;
        const bool success = d.getValue("bvec2", value);
        REQUIRE(success);
        REQUIRE(value == glm::bvec2(true, false));
    }
    SECTION("vec3") {
        REQUIRE_FALSE(d.hasValue<glm::vec3>("vec3"));
        d.setValue("vec3", glm::vec3(1.f, 2.f, 3.f));
        REQUIRE(d.hasValue<glm::vec3>("vec3"));
        glm::vec3 value;
        const bool success = d.getValue("vec3", value);
        REQUIRE(success);
        REQUIRE(value == glm::vec3(1.f, 2.f, 3.f));
    }
    SECTION("dvec3") {
        REQUIRE_FALSE(d.hasValue<glm::dvec3>("dvec3"));
        d.setValue("dvec3", glm::dvec3(1.0, 2.0, 3.0));
        REQUIRE(d.hasValue<glm::dvec3>("dvec3"));
        glm::dvec3 value;
        const bool success = d.getValue("dvec3", value);
        REQUIRE(success);
        REQUIRE(value == glm::dvec3(1.0, 2.0, 3.0));
    }
    SECTION("ivec3") {
        REQUIRE_FALSE(d.hasValue<glm::ivec3>("ivec3"));
        d.setValue("ivec3", glm::ivec3(1, 2, 3));
        REQUIRE(d.hasValue<glm::ivec3>("ivec3"));
        glm::ivec3 value;
        const bool success = d.getValue("ivec3", value);
        REQUIRE(success);
        REQUIRE(value == glm::ivec3(1, 2, 3));
    }
    SECTION("bvec3") {
        REQUIRE_FALSE(d.hasValue<glm::bvec3>("bvec3"));
        d.setValue("bvec3", glm::bvec3(true, false, true));
        REQUIRE(d.hasValue<glm::bvec3>("bvec3"));
        glm::bvec3 value;
        const bool success = d.getValue("bvec3", value);
        REQUIRE(success);
        REQUIRE(value == glm::bvec3(true, false, true));
    }
    SECTION("vec3") {
        REQUIRE_FALSE(d.hasValue<glm::vec4>("vec4"));
        d.setValue("vec4", glm::vec4(1.f, 2.f, 3.f, 4.f));
        REQUIRE(d.hasValue<glm::vec4>("vec4"));
        glm::vec4 value;
        const bool success = d.getValue("vec4", value);
        REQUIRE(success);
        REQUIRE(value == glm::vec4(1.f, 2.f, 3.f, 4.f));
    }
    SECTION("dvec4") {
        REQUIRE_FALSE(d.hasValue<glm::dvec4>("dvec4"));
        d.setValue("dvec4", glm::dvec4(1.0, 2.0, 3.0, 4.0));
        REQUIRE(d.hasValue<glm::dvec4>("dvec4"));
        glm::dvec4 value;
        const bool success = d.getValue("dvec4", value);
        REQUIRE(success);
        REQUIRE(value == glm::dvec4(1.0, 2.0, 3.0, 4.0));
    }
    SECTION("ivec4") {
        REQUIRE_FALSE(d.hasValue<glm::ivec4>("ivec4"));
        d.setValue("ivec4", glm::ivec4(1, 2, 3, 4));
        REQUIRE(d.hasValue<glm::ivec4>("ivec4"));
        glm::ivec4 value;
        const bool success = d.getValue("ivec4", value);
        REQUIRE(success);
        REQUIRE(value == glm::ivec4(1, 2, 3, 4));
    }
    SECTION("bvec4") {
        REQUIRE_FALSE(d.hasValue<glm::bvec4>("bvec4"));
        d.setValue("bvec4", glm::bvec4(true, false, true, false));
        REQUIRE(d.hasValue<glm::bvec4>("bvec4"));
        glm::bvec4 value;
        const bool success = d.getValue("bvec4", value);
        REQUIRE(success);
        REQUIRE(value == glm::bvec4(true, false, true, false));
    }
    SECTION("mat2x2") {
        REQUIRE_FALSE(d.hasValue<glm::mat2x2>("mat2x2"));
        d.setValue("mat2x2", glm::mat2x2(1.f, 2.f, 3.f, 4.f));
        REQUIRE(d.hasValue<glm::mat2x2>("mat2x2"));
        glm::mat2x2 value;
        const bool success = d.getValue("mat2x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat2x2(1.f, 2.f, 3.f, 4.f));
    }
    SECTION("mat2x3") {
        REQUIRE_FALSE(d.hasValue<glm::mat2x3>("mat2x3"));
        d.setValue("mat2x3", glm::mat2x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f));
        REQUIRE(d.hasValue<glm::mat2x3>("mat2x3"));
        glm::mat2x3 value;
        const bool success = d.getValue("mat2x3", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat2x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f));
    }
    SECTION("mat2x4") {
        REQUIRE_FALSE(d.hasValue<glm::mat2x4>("mat2x4"));
        d.setValue("mat2x4", glm::mat2x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f));
        REQUIRE(d.hasValue<glm::mat2x4>("mat2x4"));
        glm::mat2x4 value;
        const bool success = d.getValue("mat2x4", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat2x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f));
    }
    SECTION("mat3x2") {
        REQUIRE_FALSE(d.hasValue<glm::mat3x2>("mat3x2"));
        d.setValue("mat3x2", glm::mat3x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f));
        REQUIRE(d.hasValue<glm::mat3x2>("mat3x2"));
        glm::mat3x2 value;
        const bool success = d.getValue("mat3x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat3x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f));
    }
    SECTION("mat3x3") {
        REQUIRE_FALSE(d.hasValue<glm::mat3x3>("mat3x3"));
        d.setValue("mat3x3", glm::mat3x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f));
        REQUIRE(d.hasValue<glm::mat3x3>("mat3x3"));
        glm::mat3x3 value;
        const bool success = d.getValue("mat3x3", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat3x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f));
    }
    SECTION("mat3x4") {
        REQUIRE_FALSE(d.hasValue<glm::mat3x4>("mat3x4"));
        d.setValue("mat3x4", glm::mat3x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f,
                                           10.f, 11.f, 12.f));
        REQUIRE(d.hasValue<glm::mat3x4>("mat3x4"));
        glm::mat3x4 value;
        const bool success = d.getValue("mat3x4", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            glm::mat3x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f)
        );
    }
    SECTION("mat4x2") {
        REQUIRE_FALSE(d.hasValue<glm::mat4x2>("mat4x2"));
        d.setValue("mat4x2", glm::mat4x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f));
        REQUIRE(d.hasValue<glm::mat4x2>("mat4x2"));
        glm::mat4x2 value;
        const bool success = d.getValue("mat4x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::mat4x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f));
    }
    SECTION("mat4x3") {
        REQUIRE_FALSE(d.hasValue<glm::mat4x3>("mat4x3"));
        d.setValue("mat4x3", glm::mat4x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f));
        REQUIRE(d.hasValue<glm::mat4x3>("mat4x3"));
        glm::mat4x3 value;
        const bool success = d.getValue("mat4x3", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            glm::mat4x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f)
        );
    }
    SECTION("mat4x4") {
        REQUIRE_FALSE(d.hasValue<glm::mat4x4>("mat4x4"));
        d.setValue("mat4x4", glm::mat4x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f,
                                           10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f));
        REQUIRE(d.hasValue<glm::mat4x4>("mat4x4"));
        glm::mat4x4 value;
        const bool success = d.getValue("mat4x4", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            glm::mat4x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f,
                12.f, 13.f, 14.f, 15.f, 16.f)
        );
    }
    SECTION("dmat2x2") {
        REQUIRE_FALSE(d.hasValue<glm::dmat2x2>("dmat2x2"));
        d.setValue("dmat2x2", glm::dmat2x2(1.0, 2.0, 3.0, 4.0));
        REQUIRE(d.hasValue<glm::dmat2x2>("dmat2x2"));
        glm::dmat2x2 value;
        const bool success = d.getValue("dmat2x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat2x2(1.0, 2.0, 3.0, 4.0));
    }
    SECTION("dmat2x3") {
        REQUIRE_FALSE(d.hasValue<glm::dmat2x3>("dmat2x3"));
        d.setValue("dmat2x3", glm::dmat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0));
        REQUIRE(d.hasValue<glm::dmat2x3>("dmat2x3"));
        glm::dmat2x3 value;
        const bool success = d.getValue("dmat2x3", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0));
    }
    SECTION("dmat2x4") {
        REQUIRE_FALSE(d.hasValue<glm::dmat2x4>("dmat2x4"));
        d.setValue("dmat2x4", glm::dmat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0));
        REQUIRE(d.hasValue<glm::dmat2x4>("dmat2x4"));
        glm::dmat2x4 value;
        const bool success = d.getValue("dmat2x4", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0));
    }
    SECTION("dmat3x2") {
        REQUIRE_FALSE(d.hasValue<glm::dmat3x2>("dmat3x2"));
        d.setValue("dmat3x2", glm::dmat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0));
        REQUIRE(d.hasValue<glm::dmat3x2>("dmat3x2"));
        glm::dmat3x2 value;
        const bool success = d.getValue("dmat3x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0));
    }
    SECTION("dmat3x3") {
        REQUIRE_FALSE(d.hasValue<glm::dmat3x3>("dmat3x3"));
        d.setValue("dmat3x3",
                     glm::dmat3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0));
        REQUIRE(d.hasValue<glm::dmat3x3>("dmat3x3"));
        glm::dmat3x3 value;
        const bool success = d.getValue("dmat3x3", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0));
    }
    SECTION("dmat3x4") {
        REQUIRE_FALSE(d.hasValue<glm::dmat3x4>("dmat3x4"));
        d.setValue("dmat3x4", glm::dmat3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
                                             10.0, 11.0, 12.0));
        REQUIRE(d.hasValue<glm::dmat3x4>("dmat3x4"));
        glm::dmat3x4 value;
        const bool success = d.getValue("dmat3x4", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            glm::dmat3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0)
        );
    }
    SECTION("dmat4x2") {
        REQUIRE_FALSE(d.hasValue<glm::dmat4x2>("dmat4x2"));
        d.setValue("dmat4x2", glm::dmat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0));
        REQUIRE(d.hasValue<glm::dmat4x2>("dmat4x2"));
        glm::dmat4x2 value;
        const bool success = d.getValue("dmat4x2", value);
        REQUIRE(success);
        REQUIRE(value == glm::dmat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0));
    }
    SECTION("dmat4x3") {
        REQUIRE_FALSE(d.hasValue<glm::dmat4x3>("dmat4x3"));
        d.setValue("dmat4x3", glm::dmat4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
                                             10.0, 11.0, 12.0));
        REQUIRE(d.hasValue<glm::dmat4x3>("dmat4x3"));
        glm::dmat4x3 value;
        const bool success = d.getValue("dmat4x3", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            glm::dmat4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0)
        );
    }
    SECTION("dmat4x4") {
        REQUIRE_FALSE(d.hasValue<glm::dmat4x4>("dmat4x4"));
        d.setValue("dmat4x4", glm::dmat4x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
                                             10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
        REQUIRE(d.hasValue<glm::dmat4x4>("dmat4x4"));
        glm::dmat4x4 value;
        const bool success = d.getValue("dmat4x4", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            glm::dmat4x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
                12.0, 13.0, 14.0, 15.0, 16.0)
        );
    }
}

TEST_CASE("Dictionary: hasValue Type Cast", "[dictionary]") {
    ghoul::Dictionary d = createDefaultDictionary();
    bool success;

    success = d.hasValue<bool>("bool");
    REQUIRE(success);

    success = d.hasValue<bool>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bool>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("char");
    REQUIRE(success);

    success = d.hasValue<char>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("wchar_t");
    REQUIRE(success);

    success = d.hasValue<char>("short");
    REQUIRE(success);

    success = d.hasValue<char>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("int");
    REQUIRE(success);

    success = d.hasValue<char>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("long long");
    REQUIRE(success);

    success = d.hasValue<char>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<char>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("unsigned char");
    REQUIRE(success);

    success = d.hasValue<unsigned char>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("unsigned short");
    REQUIRE(success);

    success = d.hasValue<unsigned char>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("unsigned int");
    REQUIRE(success);

    success = d.hasValue<unsigned char>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("unsigned long long");
    REQUIRE(success);

    success = d.hasValue<unsigned char>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned char>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("char");
    REQUIRE(success);

    success = d.hasValue<wchar_t>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("wchar_t");
    REQUIRE(success);

    success = d.hasValue<wchar_t>("short");
    REQUIRE(success);

    success = d.hasValue<wchar_t>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("int");
    REQUIRE(success);

    success = d.hasValue<wchar_t>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("long long");
    REQUIRE(success);

    success = d.hasValue<wchar_t>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<wchar_t>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("char");
    REQUIRE(success);

    success = d.hasValue<short>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("wchar_t");
    REQUIRE(success);

    success = d.hasValue<short>("short");
    REQUIRE(success);

    success = d.hasValue<short>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("int");
    REQUIRE(success);

    success = d.hasValue<short>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("long long");
    REQUIRE(success);

    success = d.hasValue<short>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<short>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("unsigned char");
    REQUIRE(success);

    success = d.hasValue<unsigned short>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("unsigned short");
    REQUIRE(success);

    success = d.hasValue<unsigned short>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("unsigned int");
    REQUIRE(success);

    success = d.hasValue<unsigned short>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("unsigned long long");
    REQUIRE(success);

    success = d.hasValue<unsigned short>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned short>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("char");
    REQUIRE(success);

    success = d.hasValue<int>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("wchar_t");
    REQUIRE(success);

    success = d.hasValue<int>("short");
    REQUIRE(success);

    success = d.hasValue<int>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("int");
    REQUIRE(success);

    success = d.hasValue<int>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("long long");
    REQUIRE(success);

    success = d.hasValue<int>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<int>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("unsigned char");
    REQUIRE(success);

    success = d.hasValue<unsigned int>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("unsigned short");
    REQUIRE(success);

    success = d.hasValue<unsigned int>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("unsigned int");
    REQUIRE(success);

    success = d.hasValue<unsigned int>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("unsigned long long");
    REQUIRE(success);

    success = d.hasValue<unsigned int>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned int>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("char");
    REQUIRE(success);

    success = d.hasValue<long long>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("wchar_t");
    REQUIRE(success);

    success = d.hasValue<long long>("short");
    REQUIRE(success);

    success = d.hasValue<long long>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("int");
    REQUIRE(success);

    success = d.hasValue<long long>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("long long");
    REQUIRE(success);

    success = d.hasValue<long long>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long long>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("unsigned char");
    REQUIRE(success);

    success = d.hasValue<unsigned long long>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("unsigned short");
    REQUIRE(success);

    success = d.hasValue<unsigned long long>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("unsigned int");
    REQUIRE(success);

    success = d.hasValue<unsigned long long>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("unsigned long long");
    REQUIRE(success);

    success = d.hasValue<unsigned long long>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<unsigned long long>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("float");
    REQUIRE(success);

    success = d.hasValue<float>("double");
    REQUIRE(success);

    success = d.hasValue<float>("long double");
    REQUIRE_FALSE(success); // this is not a mistake

    success = d.hasValue<float>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<float>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("float");
    REQUIRE(success);

    success = d.hasValue<double>("double");
    REQUIRE(success);

    success = d.hasValue<double>("long double");
    REQUIRE_FALSE(success); // this is not a mistake

    success = d.hasValue<double>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<double>("dmat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("long double");
    REQUIRE(success); // this is not a mistake

    success = d.hasValue<long double>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<long double>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::vec2;

    success = d.hasValue<vec2>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("vec2");
    REQUIRE(success);

    success = d.hasValue<vec2>("dvec2");
    REQUIRE(success);

    success = d.hasValue<vec2>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec2>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::dvec2;

    success = d.hasValue<dvec2>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("vec2");
    REQUIRE(success);

    success = d.hasValue<dvec2>("dvec2");
    REQUIRE(success);

    success = d.hasValue<dvec2>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec2>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::ivec2;

    success = d.hasValue<ivec2>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("ivec2");
    REQUIRE(success);

    success = d.hasValue<ivec2>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("bvec2");
    REQUIRE(success);

    success = d.hasValue<ivec2>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec2>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::uvec2;

    success = d.hasValue<uvec2>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("uvec2");
    REQUIRE(success);

    success = d.hasValue<uvec2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec2>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::bvec2;

    success = d.hasValue<bvec2>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("bvec2");
    REQUIRE(success);

    success = d.hasValue<bvec2>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("bvec2");
    REQUIRE(success);

    success = d.hasValue<bvec2>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec2>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::vec3;

    success = d.hasValue<vec3>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("vec3");
    REQUIRE(success);

    success = d.hasValue<vec3>("dvec3");
    REQUIRE(success);

    success = d.hasValue<vec3>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec3>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::dvec3;

    success = d.hasValue<dvec3>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("vec3");
    REQUIRE(success);

    success = d.hasValue<dvec3>("dvec3");
    REQUIRE(success);

    success = d.hasValue<dvec3>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec3>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::ivec3;

    success = d.hasValue<ivec3>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("ivec3");
    REQUIRE(success);

    success = d.hasValue<ivec3>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("bvec3");
    REQUIRE(success);

    success = d.hasValue<ivec3>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec3>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::uvec3;

    success = d.hasValue<uvec3>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("uvec3");
    REQUIRE(success);

    success = d.hasValue<uvec3>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec3>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::bvec3;

    success = d.hasValue<bvec3>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("ivec3");
    REQUIRE(success);

    success = d.hasValue<bvec3>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("bvec3");
    REQUIRE(success);

    success = d.hasValue<bvec3>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec3>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::vec4;

    success = d.hasValue<vec4>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("vec4");
    REQUIRE(success);

    success = d.hasValue<vec4>("dvec4");
    REQUIRE(success);

    success = d.hasValue<vec4>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("mat2x2");
    REQUIRE(success);

    success = d.hasValue<vec4>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("dmat2x2");
    REQUIRE(success);

    success = d.hasValue<vec4>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<vec4>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::dvec4;

    success = d.hasValue<dvec4>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("vec4");
    REQUIRE(success);

    success = d.hasValue<dvec4>("dvec4");
    REQUIRE(success);

    success = d.hasValue<dvec4>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("mat2x2");
    REQUIRE(success);

    success = d.hasValue<dvec4>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("dmat2x2");
    REQUIRE(success);

    success = d.hasValue<dvec4>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dvec4>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::ivec4;

    success = d.hasValue<ivec4>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("ivec4");
    REQUIRE(success);

    success = d.hasValue<ivec4>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("bvec4");
    REQUIRE(success);

    success = d.hasValue<ivec4>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<ivec4>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::uvec4;

    success = d.hasValue<uvec4>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("ivec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("uvec4");
    REQUIRE(success);

    success = d.hasValue<uvec4>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<uvec4>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::bvec4;

    success = d.hasValue<bvec4>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("ivec4");
    REQUIRE(success);

    success = d.hasValue<bvec4>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("bvec4");
    REQUIRE(success);

    success = d.hasValue<bvec4>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<bvec4>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::mat2x2;

    success = d.hasValue<mat2x2>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("vec4");
    REQUIRE(success);

    success = d.hasValue<mat2x2>("dvec4");
    REQUIRE(success);

    success = d.hasValue<mat2x2>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("mat2x2");
    REQUIRE(success);

    success = d.hasValue<mat2x2>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("dmat2x2");
    REQUIRE(success);

    success = d.hasValue<mat2x2>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x2>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::mat2x3;

    success = d.hasValue<mat2x3>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("mat2x3");
    REQUIRE(success);

    success = d.hasValue<mat2x3>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("mat3x2");
    REQUIRE(success);

    success = d.hasValue<mat2x3>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("dmat2x3");
    REQUIRE(success);

    success = d.hasValue<mat2x3>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("dmat3x2");
    REQUIRE(success);

    success = d.hasValue<mat2x3>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x3>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::mat2x4;

    success = d.hasValue<mat2x4>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("mat2x4");
    REQUIRE(success);

    success = d.hasValue<mat2x4>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("mat4x2");
    REQUIRE(success);

    success = d.hasValue<mat2x4>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("dmat2x4");
    REQUIRE(success);

    success = d.hasValue<mat2x4>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("dmat4x2");
    REQUIRE(success);

    success = d.hasValue<mat2x4>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat2x4>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::mat3x2;

    success = d.hasValue<mat3x2>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("mat2x3");
    REQUIRE(success);

    success = d.hasValue<mat3x2>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("mat3x2");
    REQUIRE(success);

    success = d.hasValue<mat3x2>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("dmat2x3");
    REQUIRE(success);

    success = d.hasValue<mat3x2>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("dmat3x2");
    REQUIRE(success);

    success = d.hasValue<mat3x2>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x2>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::mat3x3;

    success = d.hasValue<mat3x3>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("mat3x3");
    REQUIRE(success);

    success = d.hasValue<mat3x3>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dmat3x3");
    REQUIRE(success);

    success = d.hasValue<mat3x3>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x3>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::mat3x4;

    success = d.hasValue<mat3x4>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("mat3x4");
    REQUIRE(success);

    success = d.hasValue<mat3x4>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("mat4x3");
    REQUIRE(success);

    success = d.hasValue<mat3x4>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("dmat3x4");
    REQUIRE(success);

    success = d.hasValue<mat3x4>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat3x4>("dmat4x3");
    REQUIRE(success);

    success = d.hasValue<mat3x4>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::mat4x2;

    success = d.hasValue<mat4x2>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("mat2x4");
    REQUIRE(success);

    success = d.hasValue<mat4x2>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("mat4x2");
    REQUIRE(success);

    success = d.hasValue<mat4x2>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("dmat2x4");
    REQUIRE(success);

    success = d.hasValue<mat4x2>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("dmat4x2");
    REQUIRE(success);

    success = d.hasValue<mat4x2>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x2>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::mat4x3;

    success = d.hasValue<mat4x3>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("mat3x4");
    REQUIRE(success);

    success = d.hasValue<mat4x3>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("mat4x3");
    REQUIRE(success);

    success = d.hasValue<mat4x3>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("dmat3x4");
    REQUIRE(success);

    success = d.hasValue<mat4x3>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x3>("dmat4x3");
    REQUIRE(success);

    success = d.hasValue<mat4x3>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::mat4x4;

    success = d.hasValue<mat4x4>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("mat4x4");
    REQUIRE(success);

    success = d.hasValue<mat4x4>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<mat4x4>("dmat4x4");
    REQUIRE(success);

    using glm::dmat2x2;

    success = d.hasValue<dmat2x2>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("vec4");
    REQUIRE(success);

    success = d.hasValue<dmat2x2>("dvec4");
    REQUIRE(success);

    success = d.hasValue<dmat2x2>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("mat2x2");
    REQUIRE(success);

    success = d.hasValue<dmat2x2>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("dmat2x2");
    REQUIRE(success);

    success = d.hasValue<dmat2x2>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x2>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::dmat2x3;

    success = d.hasValue<dmat2x3>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("mat2x3");
    REQUIRE(success);

    success = d.hasValue<dmat2x3>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("mat3x2");
    REQUIRE(success);

    success = d.hasValue<dmat2x3>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("dmat2x3");
    REQUIRE(success);

    success = d.hasValue<dmat2x3>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("dmat3x2");
    REQUIRE(success);

    success = d.hasValue<dmat2x3>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x3>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::dmat2x4;

    success = d.hasValue<dmat2x4>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("mat2x4");
    REQUIRE(success);

    success = d.hasValue<dmat2x4>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("mat4x2");
    REQUIRE(success);

    success = d.hasValue<dmat2x4>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("dmat2x4");
    REQUIRE(success);

    success = d.hasValue<dmat2x4>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("dmat4x2");
    REQUIRE(success);

    success = d.hasValue<dmat2x4>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat2x4>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::dmat3x2;

    success = d.hasValue<dmat3x2>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("mat2x3");
    REQUIRE(success);

    success = d.hasValue<dmat3x2>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("mat3x2");
    REQUIRE(success);

    success = d.hasValue<dmat3x2>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("dmat2x3");
    REQUIRE(success);

    success = d.hasValue<dmat3x2>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("dmat3x2");
    REQUIRE(success);

    success = d.hasValue<dmat3x2>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x2>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::dmat3x3;

    success = d.hasValue<dmat3x3>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("mat3x3");
    REQUIRE(success);

    success = d.hasValue<dmat3x3>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dmat3x3");
    REQUIRE(success);

    success = d.hasValue<dmat3x3>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x3>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::dmat3x4;

    success = d.hasValue<dmat3x4>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("mat3x4");
    REQUIRE(success);

    success = d.hasValue<dmat3x4>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("mat4x3");
    REQUIRE(success);

    success = d.hasValue<dmat3x4>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("dmat3x4");
    REQUIRE(success);

    success = d.hasValue<dmat3x4>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat3x4>("dmat4x3");
    REQUIRE(success);

    success = d.hasValue<dmat3x4>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::dmat4x2;

    success = d.hasValue<dmat4x2>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("mat2x4");
    REQUIRE(success);

    success = d.hasValue<dmat4x2>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("mat4x2");
    REQUIRE(success);

    success = d.hasValue<dmat4x2>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("dmat2x4");
    REQUIRE(success);

    success = d.hasValue<dmat4x2>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("dmat4x2");
    REQUIRE(success);

    success = d.hasValue<dmat4x2>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x2>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::dmat4x3;

    success = d.hasValue<dmat4x3>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("mat3x4");
    REQUIRE(success);

    success = d.hasValue<dmat4x3>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("mat4x3");
    REQUIRE(success);

    success = d.hasValue<dmat4x3>("mat4x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("dmat3x4");
    REQUIRE(success);

    success = d.hasValue<dmat4x3>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x3>("dmat4x3");
    REQUIRE(success);

    success = d.hasValue<dmat4x3>("dmat4x4");
    REQUIRE_FALSE(success);

    using glm::dmat4x4;

    success = d.hasValue<dmat4x4>("bool");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("unsigned char");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("wchar_t");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("unsigned short");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("unsigned int");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("unsigned long long");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("float");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("long double");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dictionary");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("vec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("uvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("bvec2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("vec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("ivec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("uvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("bvec3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("vec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("ivec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("uvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("bvec4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("mat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("mat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("mat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("mat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("mat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("mat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("mat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("mat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("mat4x4");
    REQUIRE(success);

    success = d.hasValue<dmat4x4>("dmat2x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dmat2x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dmat2x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dmat3x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dmat3x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dmat3x4");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dmat4x2");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dmat4x3");
    REQUIRE_FALSE(success);

    success = d.hasValue<dmat4x4>("dmat4x4");
    REQUIRE(success);
}

TEST_CASE("Dictionary: Conversion From Dictionary", "[dictionary]") {
    ghoul::Dictionary dict1ElemInteger = { { "0", 1 } };
    ghoul::Dictionary dict1ElemUnsignedInteger = { { "0", 1u } };
    ghoul::Dictionary dict1ElemFloat = { { "0", 1.f } };
    ghoul::Dictionary dict2ElemInteger = { { "0", 1 }, { "1", 2 } };
    ghoul::Dictionary dict2ElemUnsignedInteger = { { "0", 1u }, { "1", 2u } };
    ghoul::Dictionary dict2ElemFloat = { { "0", 1.f }, { "1", 2.f } };
    ghoul::Dictionary dict3ElemInteger = { { "0", 1 }, { "1", 2 }, { "2", 3 } };
    ghoul::Dictionary dict3ElemUnsignedInteger = {
        { "0", 1u }, { "1", 2u }, { "2", 3u }
    };
    ghoul::Dictionary dict3ElemFloat = { { "0", 1.f }, { "1", 2.f }, { "2", 3.f } };
    ghoul::Dictionary dict4ElemInteger = {
        { "0", 1 }, { "1", 2 }, { "2", 3 }, { "3", 4 }
    };
    ghoul::Dictionary dict4ElemUnsignedInteger = {
        {"0", 1u }, { "1", 2u }, { "2", 3u }, { "3", 4u }
    };
    ghoul::Dictionary dict4ElemFloat = {
        { "0", 1.f }, { "1", 2.f }, { "2", 3.f }, { "3", 4.f }
    };
    ghoul::Dictionary dict6ElemInteger = {
        { "0", 1 }, { "1", 2 }, { "2", 3 }, { "3", 4 }, { "4", 5 }, { "5", 6 }
    };
    ghoul::Dictionary dict6ElemUnsignedInteger = {
        { "0", 1u }, { "1", 2u }, { "2", 3u }, { "3", 4u }, { "4", 5u }, { "5", 6u }
    };
    ghoul::Dictionary dict6ElemFloat = {
        { "0", 1.f }, { "1", 2.f }, { "2", 3.f }, { "3", 4.f }, { "4", 5.f }, { "5", 6.f }
    };
    ghoul::Dictionary dict8ElemInteger = {
        { "0", 1 }, { "1", 2 }, { "2", 3 }, { "3", 4 }, { "4", 5 }, { "5", 6 },
        { "6", 7 }, {"7", 8 }
    };
    ghoul::Dictionary dict8ElemUnsignedInteger = {
        { "0", 1u }, { "1", 2u }, {"2", 3u }, { "3", 4u }, { "4", 5u }, { "5", 6u },
        { "6", 7u }, { "7", 8u }
    };
    ghoul::Dictionary dict8ElemFloat = {
        { "0", 1.f }, { "1", 2.f }, { "2", 3.f }, { "3", 4.f }, { "4", 5.f },
        { "5", 6.f }, { "6", 7.f }, { "7", 8.f }
    };
    ghoul::Dictionary dict9ElemInteger = {
        { "0", 1 }, { "1", 2 }, { "2", 3 }, { "3", 4 }, { "4", 5 }, { "5",  6},
        { "6", 7 }, { "7", 8 }, { "8", 9 }
    };
    ghoul::Dictionary dict9ElemUnsignedInteger = {
        { "0", 1u }, { "1", 2u }, { "2", 3u }, { "3", 4u }, { "4", 5u },
        { "5", 6u }, { "6", 7u }, { "7", 8u }, { "8", 9u }
    };
    ghoul::Dictionary dict9ElemFloat = {
        { "0", 1.f }, { "1", 2.f }, { "2", 3.f }, { "3", 4.f }, { "4", 5.f },
        { "5", 6.f }, { "6", 7.f }, { "7", 8.f }, { "8", 9.f }
    };
    ghoul::Dictionary dict12ElemInteger = {
        { "0", 1 }, { "1", 2 }, { "2", 3 }, { "3", 4 }, { "4", 5 }, { "5", 6 },
        { "6",  7}, { "7", 8 }, { "8", 9 }, { "9", 10 }, { "10", 11 }, { "11", 12 }
    };
    ghoul::Dictionary dict12ElemUnsignedInteger = {
        { "0", 1u }, { "1", 2u }, { "2", 3u }, { "3", 4u }, { "4", 5u }, { "5", 6u },
        { "6", 7u }, { "7", 8u }, { "8", 9u }, { "9", 10u }, { "10", 11u }, { "11", 12u }
    };
    ghoul::Dictionary dict12ElemFloat = {
        { "0", 1.f }, { "1", 2.f }, { "2", 3.f }, { "3", 4.f }, { "4", 5.f },
        { "5", 6.f }, { "6", 7.f }, { "7", 8.f }, { "8", 9.f }, { "9", 10.f },
        { "10", 11.f }, { "11", 12.f }
    };
    ghoul::Dictionary dict16ElemInteger = {
        { "0", 1}, { "1",  2 },{ "2", 3 }, { "3", 4 }, { "4", 5 }, { "5", 6 },
        { "6", 7 }, { "7", 8 }, { "8", 9 }, { "9", 10 }, { "10", 11 }, { "11", 12 },
        { "12", 13 }, { "13", 14 }, { "14", 15 }, { "15", 16 }
    };
    ghoul::Dictionary dict16ElemUnsignedInteger = {
        { "0", 1u }, { "1", 2u }, { "2", 3u }, { "3", 4u }, { "4", 5u }, { "5", 6u },
        { "6", 7u }, { "7", 8u }, { "8", 9u }, { "9", 10u }, { "10", 11u }, { "11", 12u },
        { "12", 13u }, { "13", 14u }, { "14", 15u },{ "15", 16u }
    };
    ghoul::Dictionary dict16ElemFloat = {
        { "0", 1.f }, { "1", 2.f },{ "2", 3.f }, { "3", 4.f }, { "4", 5.f }, { "5", 6.f },
        { "6", 7.f }, { "7", 8.f }, { "8", 9.f }, { "9", 10.f }, { "10", 11.f },
        { "11", 12.f }, { "12", 13.f }, { "13", 14.f }, { "14", 15.f }, { "15", 16.f }
    };

    ghoul::Dictionary d = {
        { "1int", dict1ElemInteger },
        { "1uint", dict1ElemUnsignedInteger },
        { "1float", dict1ElemFloat},
        { "2int", dict2ElemInteger },
        { "2uint", dict2ElemUnsignedInteger },
        { "2float", dict2ElemFloat },
        { "3int", dict3ElemInteger },
        { "3uint", dict3ElemUnsignedInteger },
        { "3float", dict3ElemFloat },
        { "4int", dict4ElemInteger },
        { "4uint", dict4ElemUnsignedInteger },
        { "4float", dict4ElemFloat },
        { "6int", dict6ElemInteger },
        { "6uint", dict6ElemUnsignedInteger },
        { "6float", dict6ElemFloat },
        { "8int", dict8ElemInteger },
        { "8uint", dict8ElemUnsignedInteger },
        { "8float", dict8ElemFloat },
        { "9int", dict9ElemInteger },
        { "9uint", dict9ElemUnsignedInteger },
        { "9float", dict9ElemFloat },
        { "12int", dict12ElemInteger },
        { "12uint", dict12ElemUnsignedInteger },
        { "12float", dict12ElemFloat },
        { "16int", dict16ElemInteger },
        { "16uint", dict16ElemUnsignedInteger },
        { "16float", dict16ElemFloat }
    };

    bool success;
    success = d.hasValue<bool>("1int");
    REQUIRE_FALSE(success);
    success = d.hasValue<bool>("1uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<bool>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<bool>("2int");
    REQUIRE_FALSE(success);
    {
        bool value;
        success = d.getValue("1int", value);
        REQUIRE_FALSE(success);
    }

    success = d.hasValue<char>("1int");
    REQUIRE(success);
    success = d.hasValue<char>("1uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<char>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<char>("2int");
    REQUIRE_FALSE(success);
    {
        char value;
        success = d.getValue("1int", value);
        REQUIRE(success);
        REQUIRE(value == char(1));
    }

    success = d.hasValue<signed char>("1int");
    REQUIRE(success);
    success = d.hasValue<signed char>("1uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<signed char>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<signed char>("2int");
    REQUIRE_FALSE(success);
    {
        signed char value;
        success = d.getValue("1int", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<signed char>(1));
    }
    success = d.hasValue<unsigned char>("1int");
    REQUIRE_FALSE(success);
    success = d.hasValue<unsigned char>("1uint");
    REQUIRE(success);
    success = d.hasValue<unsigned char>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<unsigned char>("2int");
    REQUIRE_FALSE(success);
    {
        unsigned char value;
        success = d.getValue("1uint", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned char>(1));
    }

    success = d.hasValue<wchar_t>("1int");
    REQUIRE(success);
    success = d.hasValue<wchar_t>("1uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<wchar_t>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<wchar_t>("2int");
    REQUIRE_FALSE(success);
    {
        wchar_t value;
        success = d.getValue("1int", value);
        REQUIRE(success);
        REQUIRE(value == wchar_t(1));
    }

    success = d.hasValue<short>("1int");
    REQUIRE(success);
    success = d.hasValue<short>("1uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<short>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<short>("2int");
    REQUIRE_FALSE(success);
    {
        short value;
        success = d.getValue("1int", value);
        REQUIRE(success);
        REQUIRE(value == short(1));
    }

    success = d.hasValue<unsigned short>("1int");
    REQUIRE_FALSE(success);
    success = d.hasValue<unsigned short>("1uint");
    REQUIRE(success);
    success = d.hasValue<unsigned short>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<unsigned short>("2int");
    REQUIRE_FALSE(success);
    {
        unsigned short value;
        success = d.getValue("1uint", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned short>(1));
    }

    success = d.hasValue<int>("1int");
    REQUIRE(success);
    success = d.hasValue<int>("1uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<int>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<int>("2int");
    REQUIRE_FALSE(success);
    {
        int value;
        success = d.getValue("1int", value);
        REQUIRE(success);
        REQUIRE(value == int(1));
    }

    success = d.hasValue<unsigned int>("1int");
    REQUIRE_FALSE(success);
    success = d.hasValue<unsigned int>("1uint");
    REQUIRE(success);
    success = d.hasValue<unsigned int>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<unsigned int>("2int");
    REQUIRE_FALSE(success);
    {
        unsigned int value;
        success = d.getValue("1uint", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned int>(1));
    }

    success = d.hasValue<long long>("1int");
    REQUIRE(success);
    success = d.hasValue<long long>("1uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<long long>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<long long>("2int");
    REQUIRE_FALSE(success);
    {
        long long value;
        success = d.getValue("1int", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<long long>(1));
    }

    success = d.hasValue<unsigned long long>("1int");
    REQUIRE_FALSE(success);
    success = d.hasValue<unsigned long long>("1uint");
    REQUIRE(success);
    success = d.hasValue<unsigned long long>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<unsigned long long>("2int");
    REQUIRE_FALSE(success);
    {
        unsigned long long value;
        success = d.getValue("1uint", value);
        REQUIRE(success);
        REQUIRE(value == static_cast<unsigned long long>(1));
    }

    success = d.hasValue<float>("1int");
    REQUIRE_FALSE(success);
    success = d.hasValue<float>("1uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<float>("1float");
    REQUIRE(success);
    success = d.hasValue<float>("2int");
    REQUIRE_FALSE(success);
    {
        float value;
        success = d.getValue("1float", value);
        REQUIRE(success);
        REQUIRE(value == float(1));
    }

    success = d.hasValue<double>("1int");
    REQUIRE_FALSE(success);
    success = d.hasValue<double>("1uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<double>("1float");
    REQUIRE(success);
    success = d.hasValue<double>("2int");
    REQUIRE_FALSE(success);
    {
        double value;
        success = d.getValue("1float", value);
        REQUIRE(success);
        REQUIRE(value == double(1));
    }

    using glm::vec2;

    success = d.hasValue<vec2>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<vec2>("2int");
    REQUIRE_FALSE(success);
    success = d.hasValue<vec2>("2uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<vec2>("2float");
    REQUIRE(success);
    {
        vec2 value;
        success = d.getValue("2float", value);
        REQUIRE(success);
        REQUIRE(value == vec2(1.f, 2.f));
    }

    using glm::dvec2;

    success = d.hasValue<dvec2>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dvec2>("2int");
    REQUIRE_FALSE(success);
    success = d.hasValue<dvec2>("2uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dvec2>("2float");
    REQUIRE(success);
    {
        dvec2 value;
        success = d.getValue("2float", value);
        REQUIRE(success);
        REQUIRE(value == dvec2(1.0, 2.0));
    }

    using glm::ivec2;

    success = d.hasValue<ivec2>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<ivec2>("2int");
    REQUIRE(success);
    success = d.hasValue<ivec2>("2uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<ivec2>("2float");
    REQUIRE_FALSE(success);
    {
        ivec2 value;
        success = d.getValue("2int", value);
        REQUIRE(success);
        REQUIRE(value == ivec2(1, 2));
    }

    using glm::uvec2;

    success = d.hasValue<uvec2>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<uvec2>("2int");
    REQUIRE_FALSE(success);
    success = d.hasValue<uvec2>("2uint");
    REQUIRE(success);
    success = d.hasValue<uvec2>("2float");
    REQUIRE_FALSE(success);
    {
        uvec2 value;
        success = d.getValue("2uint", value);
        REQUIRE(success);
        REQUIRE(value == uvec2(1, 2));
    }

    using glm::bvec2;

    success = d.hasValue<bvec2>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<bvec2>("2int");
    REQUIRE(success);
    success = d.hasValue<bvec2>("2uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<bvec2>("2float");
    REQUIRE_FALSE(success);
    {
        bvec2 value;
        success = d.getValue("2int", value);
        REQUIRE(success);
        REQUIRE(value.x);
        REQUIRE(value.y);
    }

    using glm::vec3;

    success = d.hasValue<vec3>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<vec3>("3int");
    REQUIRE_FALSE(success);
    success = d.hasValue<vec3>("3uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<vec3>("3float");
    REQUIRE(success);
    {
        vec3 value;
        success = d.getValue("3float", value);
        REQUIRE(success);
        REQUIRE(value == vec3(1.f, 2.f, 3.f));
    }

    using glm::dvec3;

    success = d.hasValue<dvec3>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dvec3>("3int");
    REQUIRE_FALSE(success);
    success = d.hasValue<dvec3>("3uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dvec3>("3float");
    REQUIRE(success);
    {
        dvec3 value;
        success = d.getValue("3float", value);
        REQUIRE(success);
        REQUIRE(value == dvec3(1.0, 2.0, 3.0));
    }

    using glm::ivec3;

    success = d.hasValue<ivec3>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<ivec3>("3int");
    REQUIRE(success);
    success = d.hasValue<ivec3>("3uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<ivec3>("3float");
    REQUIRE_FALSE(success);
    {
        ivec3 value;
        success = d.getValue("3int", value);
        REQUIRE(success);
        REQUIRE(value == ivec3(1, 2, 3));
    }

    using glm::uvec3;

    success = d.hasValue<uvec3>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<uvec3>("3int");
    REQUIRE_FALSE(success);
    success = d.hasValue<uvec3>("3uint");
    REQUIRE(success);
    success = d.hasValue<uvec3>("3float");
    REQUIRE_FALSE(success);
    {
        uvec3 value;
        success = d.getValue("3uint", value);
        REQUIRE(success);
        REQUIRE(value == uvec3(1, 2, 3));
    }

    using glm::bvec3;

    success = d.hasValue<bvec3>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<bvec3>("3int");
    REQUIRE(success);
    success = d.hasValue<bvec3>("3uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<bvec3>("3float");
    REQUIRE_FALSE(success);
     {
         bvec3 value;
         success = d.getValue("3int", value);
         REQUIRE(success);
         REQUIRE(value.x);
         REQUIRE(value.y);
         REQUIRE(value.z);
     }

    using glm::vec4;

    success = d.hasValue<vec4>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<vec4>("4int");
    REQUIRE_FALSE(success);
    success = d.hasValue<vec4>("4uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<vec4>("4float");
    REQUIRE(success);
    {
        vec4 value;
        success = d.getValue("4float", value);
        REQUIRE(success);
        REQUIRE(value == vec4(1.f, 2.f, 3.f, 4.f));
    }

    using glm::dvec4;

    success = d.hasValue<dvec4>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dvec4>("4int");
    REQUIRE_FALSE(success);
    success = d.hasValue<dvec4>("4uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dvec4>("4float");
    REQUIRE(success);
    {
        dvec4 value;
        success = d.getValue("4float", value);
        REQUIRE(success);
        REQUIRE(value == dvec4(1.0, 2.0, 3.0, 4.0));
    }

    using glm::ivec4;

    success = d.hasValue<ivec4>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<ivec4>("4int");
    REQUIRE(success);
    success = d.hasValue<ivec4>("4uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<ivec4>("4float");
    REQUIRE_FALSE(success);
    {
        ivec4 value;
        success = d.getValue("4int", value);
        REQUIRE(success);
        REQUIRE(value == ivec4(1, 2, 3, 4));
    }

    using glm::uvec4;

    success = d.hasValue<uvec4>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<uvec4>("4int");
    REQUIRE_FALSE(success);
    success = d.hasValue<uvec4>("4uint");
    REQUIRE(success);
    success = d.hasValue<uvec4>("4float");
    REQUIRE_FALSE(success);
    {
        uvec4 value;
        success = d.getValue("4uint", value);
        REQUIRE(success);
        REQUIRE(value == uvec4(1, 2, 3, 4));
    }

    using glm::bvec4;

    success = d.hasValue<bvec4>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<bvec4>("4int");
    REQUIRE(success);
    success = d.hasValue<bvec4>("4uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<bvec4>("4float");
    REQUIRE_FALSE(success);
     {
         bvec4 value;
         success = d.getValue("4int", value);
         REQUIRE(success);
         REQUIRE(value.x);
         REQUIRE(value.y);
         REQUIRE(value.z);
         REQUIRE(value.w);
     }

    using glm::mat2x2;

    success = d.hasValue<mat2x2>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat2x2>("4int");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat2x2>("4uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat2x2>("4float");
    REQUIRE(success);
    {
        mat2x2 value;
        success = d.getValue("4float", value);
        REQUIRE(success);
        REQUIRE(value == mat2x2(1.f, 2.f, 3.f, 4.f));
    }

    using glm::mat2x3;

    success = d.hasValue<mat2x3>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat2x3>("6int");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat2x3>("6uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat2x3>("6float");
    REQUIRE(success);
    {
        mat2x3 value;
        success = d.getValue("6float", value);
        REQUIRE(success);
        REQUIRE(value == mat2x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f));
    }

    using glm::mat2x4;

    success = d.hasValue<mat2x4>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat2x4>("8int");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat2x4>("8uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat2x4>("8float");
    REQUIRE(success);
    {
        mat2x4 value;
        success = d.getValue("8float", value);
        REQUIRE(success);
        REQUIRE(value == mat2x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f));
    }

    using glm::mat3x2;

    success = d.hasValue<mat3x2>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat3x2>("6int");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat3x2>("6uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat3x2>("6float");
    REQUIRE(success);
    {
        mat3x2 value;
        success = d.getValue("6float", value);
        REQUIRE(success);
        REQUIRE(value == mat3x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f));
    }

    using glm::mat3x3;

    success = d.hasValue<mat3x3>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat3x3>("9int");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat3x3>("9uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat3x3>("9float");
    REQUIRE(success);
    {
        mat3x3 value;
        success = d.getValue("9float", value);
        REQUIRE(success);
        REQUIRE(value == mat3x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f));
    }

    using glm::mat3x4;

    success = d.hasValue<mat3x4>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat3x4>("12int");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat3x4>("12uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat3x4>("12float");
    REQUIRE(success);
    {
        mat3x4 value;
        success = d.getValue("12float", value);
        REQUIRE(success);
        REQUIRE(
            value == mat3x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f)
        );
    }

    using glm::mat4x2;

    success = d.hasValue<mat4x2>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat4x2>("8int");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat4x2>("8uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat4x2>("8float");
    REQUIRE(success);
    {
        mat4x2 value;
        success = d.getValue("8float", value);
        REQUIRE(success);
        REQUIRE(value == mat4x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f));
    }

    using glm::mat4x3;

    success = d.hasValue<mat4x3>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat4x3>("12int");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat4x3>("12uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat4x3>("12float");
    REQUIRE(success);
    {
        mat4x3 value;
        success = d.getValue("12float", value);
        REQUIRE(success);
        REQUIRE(
            value == mat4x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f)
        );
    }

    using glm::mat4x4;

    success = d.hasValue<mat4x4>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat4x4>("16int");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat4x4>("16uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat4x4>("16float");
    REQUIRE(success);
    {
        mat4x4 value;
        success = d.getValue("16float", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            mat4x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f,
                13.f, 14.f, 15.f, 16.f)
        );
    }

    using glm::dmat2x2;

    success = d.hasValue<dmat2x2>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat2x2>("4int");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat2x2>("4uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat2x2>("4float");
    REQUIRE(success);
    {
        dmat2x2 value;
        success = d.getValue("4float", value);
        REQUIRE(success);
        REQUIRE(value == dmat2x2(1.0, 2.0, 3.0, 4.0));
    }

    using glm::dmat2x3;

    success = d.hasValue<dmat2x3>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat2x3>("6int");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat2x3>("6uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat2x3>("6float");
    REQUIRE(success);
    {
        dmat2x3 value;
        success = d.getValue("6float", value);
        REQUIRE(success);
        REQUIRE(value == dmat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0));
    }

    using glm::dmat2x4;

    success = d.hasValue<dmat2x4>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat2x4>("8int");
    REQUIRE_FALSE(success);
    success = d.hasValue<mat2x4>("8uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat2x4>("8float");
    REQUIRE(success);
    {
        dmat2x4 value;
        success = d.getValue("8float", value);
        REQUIRE(success);
        REQUIRE(value == dmat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0));
    }

    using glm::dmat3x2;

    success = d.hasValue<dmat3x2>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat3x2>("6int");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat3x2>("6uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat3x2>("6float");
    REQUIRE(success);
    {
        dmat3x2 value;
        success = d.getValue("6float", value);
        REQUIRE(success);
        REQUIRE(value == dmat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0));
    }

    using glm::dmat3x3;

    success = d.hasValue<dmat3x3>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat3x3>("9int");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat3x3>("9uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat3x3>("9float");
    REQUIRE(success);
    {
        dmat3x3 value;
        success = d.getValue("9float", value);
        REQUIRE(success);
        REQUIRE(value == dmat3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0));
    }

    using glm::dmat3x4;

    success = d.hasValue<dmat3x4>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat3x4>("12int");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat3x4>("12uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat3x4>("12float");
    REQUIRE(success);
    {
        dmat3x4 value;
        success = d.getValue("12float", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            dmat3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0)
        );
    }

    using glm::dmat4x2;

    success = d.hasValue<dmat4x2>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat4x2>("8int");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat4x2>("8uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat4x2>("8float");
    REQUIRE(success);
    {
        dmat4x2 value;
        success = d.getValue("8float", value);
        REQUIRE(success);
        REQUIRE(value == dmat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0));
    }

    using glm::dmat4x3;

    success = d.hasValue<dmat4x3>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat4x3>("12int");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat4x3>("12uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat4x3>("12float");
    REQUIRE(success);
    {
        dmat4x3 value;
        success = d.getValue("12float", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            dmat4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0)
        );
    }

    using glm::dmat4x4;

    success = d.hasValue<dmat4x4>("1float");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat4x4>("16int");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat4x4>("16uint");
    REQUIRE_FALSE(success);
    success = d.hasValue<dmat4x4>("16float");
    REQUIRE(success);
    {
        dmat4x4 value;
        success = d.getValue("16float", value);
        REQUIRE(success);
        REQUIRE(
            value ==
            dmat4x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0,
                13.0, 14.0, 15.0, 16.0)
        );
    }
}

TEST_CASE("Dictionary: Recursive Access HasValue", "[dictionary]") {
    ghoul::Dictionary d = { { "a", 1 } };
    ghoul::Dictionary e = d;
    ghoul::Dictionary f = d;
    ghoul::Dictionary g = d;
    f.setValue("g", g);
    e.setValue("f", f);
    d.setValue("e", e);

    REQUIRE(d.hasValue<int>("a"));
    REQUIRE(d.hasValue<ghoul::Dictionary>("e"));
    REQUIRE(d.hasValue<int>("e.a"));
    REQUIRE(d.hasValue<ghoul::Dictionary>("e.f"));
    REQUIRE(d.hasValue<int>("e.f.a"));
    REQUIRE(d.hasValue<ghoul::Dictionary>("e.f.g"));
    REQUIRE(d.hasValue<int>("e.f.g.a"));

    REQUIRE_FALSE(d.hasValue<int>("e.g"));
}

TEST_CASE("Dictionary: Recursive Access GetValue", "[dictionary]") {
    ghoul::Dictionary d = { { "a", 1 } };
    ghoul::Dictionary e = d;
    ghoul::Dictionary f = d;
    ghoul::Dictionary g = d;
    f.setValue("g", g);
    e.setValue("f", f);
    d.setValue("e", e);

    int value;
    ghoul::Dictionary dictValue;
    bool success;
    success = d.getValue("a", value);
    REQUIRE(success);
    REQUIRE(value == 1);
    success = d.getValue("e", dictValue);
    REQUIRE(success);

    success = d.getValue("e.a", value);
    REQUIRE(success);
    REQUIRE(value == 1);
    success = d.getValue("e.f", dictValue);
    REQUIRE(success);

    success = d.getValue("e.f.a", value);
    REQUIRE(success);
    REQUIRE(value == 1);
    success = d.getValue("e.f.g", dictValue);
    REQUIRE(success);

    success = d.getValue("e.f.g.a", value);
    REQUIRE(success);
    REQUIRE(value == 1);

    // false values
    success = d.getValue("e.g", value);
    REQUIRE_FALSE(success);
}

TEST_CASE("Dictionary: Recursive Access Set Value", "[dictionary]") {
    ghoul::Dictionary d = { { "a", 1 } };
    ghoul::Dictionary e = d;
    ghoul::Dictionary f = d;
    ghoul::Dictionary g = d;
    f.setValue("g", g);
    e.setValue("f", f);
    d.setValue("e", e);

    int value;
    bool success;
    success = d.getValue("e.f.g.a", value);
    REQUIRE(success);
    REQUIRE(value == 1);

    REQUIRE_NOTHROW(d.setValue("e.f.g.b", 2));
    success = d.getValue("e.f.g.b", value);
    REQUIRE(success);
    REQUIRE(value == 2);

    // false values
    REQUIRE_THROWS_AS(d.setValue("e.g.a", 1), ghoul::Dictionary::KeyError);
}
