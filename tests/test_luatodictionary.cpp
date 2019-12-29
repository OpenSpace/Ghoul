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

#include <ghoul/glm.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/lua/lua_helper.h>
#include <ghoul/misc/dictionary.h>

TEST_CASE("LuaToDictionary: Load Test1 cfg", "[luatodictionary]") {
    // test1.cfg
    // return {
    //    t = 1
    //}

    ghoul::Dictionary d;
    std::string test1 = absPath("${UNIT_TEST}/luatodictionary/test1.cfg");
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(test1, d));
    REQUIRE(d.size() == 1);

    double value;
    bool success = d.getValue("t", value);
    REQUIRE(success);
    REQUIRE(value == 1.0);
}

TEST_CASE("LuaToDictionary: Load Test2 cfg", "[luatodictionary]") {
    // test2.cfg
    //return{
    //    t = 2,
    //    s = { a = "a", b = "b", a1 = "1" }
    //}

    ghoul::Dictionary d;
    std::string test2 = absPath("${UNIT_TEST}/luatodictionary/test2.cfg");
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(test2, d));

    REQUIRE(d.size() == 2);

    double value;
    bool success = d.getValue("t", value);
    REQUIRE(success);
    REQUIRE(value == 2.0);

    ghoul::Dictionary dict;
    success = d.getValue("s", dict);
    REQUIRE( success);
    REQUIRE(dict.size() == 3);

    std::string strValue;
    success = dict.getValue("a", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "a");

    success = dict.getValue("b", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "b");

    success = dict.getValue("a1", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "1");
}

TEST_CASE("LuaToDictionary: Load Test3 cfg", "[luatodictionary]") {
    // test3.cfg
    //return{
    //    s = {
    //        ["1"] = "1",
    //        ["2"] = "2",
    //        ["3"] = {
    //            a = "3a",
    //            b = "3b"
    //        }
    //    },
    //    tt = {
    //                ["1"] = 2
    //            }
    //            -- int = { 1, "1" = 2 }
    //}

    ghoul::Dictionary d;
    std::string test3 = absPath("${UNIT_TEST}/luatodictionary/test3.cfg");
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(test3, d));

    REQUIRE(d.size() == 2);

    ghoul::Dictionary dict;
    bool success = d.getValue("s", dict);
    REQUIRE(success);
    REQUIRE(dict.size() == 3);
    std::string strValue;
    success = dict.getValue("1", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "1");
    success = dict.getValue("2", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "2");

    ghoul::Dictionary dict2;
    success = dict.getValue("3", dict2);
    REQUIRE(success);
    REQUIRE(dict2.size() == 2);

    success = dict2.getValue("a", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "3a");
    success = dict2.getValue("b", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "3b");
}

TEST_CASE("LuaToDictionary: Load Test4 cfg", "[luatodictionary]") {
    // test4.cfg
    //return {
    //    n2 = { 5, 6 },
    //    n3 = { 5, 6, 7 },
    //    n4 = { 5, 6, 7, 8 },
    //    num2 = { ["1"] = 5, ["2"] = 6 },
    //    num3 = { ["1"] = 5, ["2"] = 6, ["3"] = 7 },
    //    num4 = { ["1"] = 5, ["2"] = 6, ["3"] = 7, ["4"] = 8 },
    //    xy = { x = 5, y = 6 },
    //    xyz = { x = 5, y = 6, z = 7 },
    //    xyzw = { x = 5, y = 6, z = 7, w = 8 },
    //    rg = { r = 5, g = 6 },
    //    rgb = { r = 5, g = 6, b = 7 },
    //    rgba = { r = 5, g = 6, b = 7, a = 8 },
    //    st = { s = 5, t = 6 },
    //    stp = { s = 5, t = 6, p = 7 },
    //    stpq = { s = 5, t = 6, p = 7, q = 8 },
    //    mix = { x = 1, g = 2, p = 3 },
    //    m2x2 = { 5, 6, 9, 10 },
    //    m2x3 = { 5, 6, 7, 9, 10, 11 },
    //    m2x4 = { 5, 6, 7, 8, 9, 10, 11, 12 },
    //    m3x2 = { 5, 6, 9, 10, 13, 14 },
    //    m3x3 = { 5, 6, 7, 9, 10, 11, 13, 14, 15 },
    //    m3x4 = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 },
    //    m4x2 = { 5, 6, 9, 10, 13, 14, 17, 18 },
    //    m4x3 = { 5, 6, 7, 9, 10, 11, 13, 14, 15, 17, 18, 19 },
    //    m4x4 = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 }
    //}
    ghoul::Dictionary d;
    std::string test4 = absPath("${UNIT_TEST}/luatodictionary/test4.cfg");
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(test4, d));

    REQUIRE(d.size() == 25);

    glm::vec2 vec2Value;
    bool success = d.getValue("n2", vec2Value);
    REQUIRE(success);
    success = d.getValue("num2", vec2Value);
    REQUIRE(success);
    success = d.getValue("xy", vec2Value);
    REQUIRE(success);
    success = d.getValue("rg", vec2Value);
    REQUIRE(success);
    success = d.getValue("st", vec2Value);
    REQUIRE(success);

    glm::vec3 vec3Value;
    success = d.getValue("n3", vec3Value);
    REQUIRE(success);
    success = d.getValue("num3", vec3Value);
    REQUIRE(success);
    success = d.getValue("xyz", vec3Value);
    REQUIRE(success);
    success = d.getValue("rgb", vec3Value);
    REQUIRE(success);
    success = d.getValue("stp", vec3Value);
    REQUIRE(success);
    success = d.getValue("mix", vec3Value);
    REQUIRE(success);

    glm::vec4 vec4Value;
    success = d.getValue("n4", vec4Value);
    REQUIRE(success);
    success = d.getValue("num4", vec4Value);
    REQUIRE(success);
    success = d.getValue("xyzw", vec4Value);
    REQUIRE(success);
    success = d.getValue("rgba", vec4Value);
    REQUIRE(success);
    success = d.getValue("stpq", vec4Value);
    REQUIRE(success);

    glm::mat2x2 mat2x2Value;
    success = d.getValue("m2x2", mat2x2Value);
    REQUIRE(success);

    glm::mat2x3 mat2x3Value;
    success = d.getValue("m2x3", mat2x3Value);
    REQUIRE(success);

    glm::mat2x4 mat2x4Value;
    success = d.getValue("m2x4", mat2x4Value);
    REQUIRE(success);

    glm::mat3x2 mat3x2Value;
    success = d.getValue("m3x2", mat3x2Value);
    REQUIRE(success);

    glm::mat3x3 mat3x3Value;
    success = d.getValue("m3x3", mat3x3Value);
    REQUIRE(success);

    glm::mat3x4 mat3x4Value;
    success = d.getValue("m3x4", mat3x4Value);
    REQUIRE(success);

    glm::mat4x2 mat4x2Value;
    success = d.getValue("m4x2", mat4x2Value);
    REQUIRE(success);

    glm::mat4x3 mat4x3Value;
    success = d.getValue("m4x3", mat4x3Value);
    REQUIRE(success);

    glm::mat4x4 mat4x4Value;
    success = d.getValue("m4x4", mat4x4Value);
    REQUIRE(success);
}

TEST_CASE("LuaToDictionary: Load Test1 cfg Direct", "[luatodictionary]") {
    const std::string script = R"script(
    return {
        t = 1
    }
    )script";

    ghoul::Dictionary d;
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromString(script, d));
    REQUIRE(d.size() == 1);

    double value;
    bool success = d.getValue("t", value);
    REQUIRE(success);
    REQUIRE(value == 1.0);
}

TEST_CASE("LuaToDictionary: Load Test2 cfg Direct", "[luatodictionary]") {
    const std::string script = R"script(
        return{
            t = 2,
            s = { a = "a", b = "b", a1 = "1" }
        }
    )script";
    // test2.cfg
    //return{
    //    t = 2,
    //    s = { a = "a", b = "b", a1 = "1" }
    //}

    ghoul::Dictionary d;

    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromString(script, d));
    REQUIRE(d.size() == 2);

    double value;
    bool success = d.getValue("t", value);
    REQUIRE(success);
    REQUIRE(value == 2.0);

    ghoul::Dictionary dict;
    success = d.getValue("s", dict);
    REQUIRE(success);
    REQUIRE(dict.size() == 3);

    std::string strValue;
    success = dict.getValue("a", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "a");

    success = dict.getValue("b", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "b");

    success = dict.getValue("a1", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "1");
}

TEST_CASE("LuaToDictionary: Load Test3 cfg Direct", "[luatodictionary]") {
    const std::string script = R"script(
        return {
            s = {
                ["1"] = "1",
                ["2"] = "2",
                ["3"] = {
                    a = "3a",
                    b = "3b"
                }
            },
            tt = {
                        ["1"] = 2
                    }
                    -- int = { 1, "1" = 2 }
        }
    )script";

    ghoul::Dictionary d;
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromString(script, d));
    REQUIRE(d.size() == 2);

    ghoul::Dictionary dict;
    bool success = d.getValue("s", dict);
    REQUIRE(success);
    REQUIRE(dict.size() == 3);
    std::string strValue;
    success = dict.getValue("1", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "1");
    success = dict.getValue("2", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "2");

    ghoul::Dictionary dict2;
    success = dict.getValue("3", dict2);
    REQUIRE(success);
    REQUIRE(dict2.size() == 2);

    success = dict2.getValue("a", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "3a");
    success = dict2.getValue("b", strValue);
    REQUIRE(success);
    REQUIRE(strValue == "3b");
}

TEST_CASE("LuaToDictionary: Load Test4 cfg Direct", "[luatodictionary]") {
    const std::string script = R"script(
        return {
            n2 = { 5, 6 },
            n3 = { 5, 6, 7 },
            n4 = { 5, 6, 7, 8 },
            num2 = { ["1"] = 5, ["2"] = 6 },
            num3 = { ["1"] = 5, ["2"] = 6, ["3"] = 7 },
            num4 = { ["1"] = 5, ["2"] = 6, ["3"] = 7, ["4"] = 8 },
            xy = { x = 5, y = 6 },
            xyz = { x = 5, y = 6, z = 7 },
            xyzw = { x = 5, y = 6, z = 7, w = 8 },
            rg = { r = 5, g = 6 },
            rgb = { r = 5, g = 6, b = 7 },
            rgba = { r = 5, g = 6, b = 7, a = 8 },
            st = { s = 5, t = 6 },
            stp = { s = 5, t = 6, p = 7 },
            stpq = { s = 5, t = 6, p = 7, q = 8 },
            mix = { x = 1, g = 2, p = 3 },
            m2x2 = { 5, 6, 9, 10 },
            m2x3 = { 5, 6, 7, 9, 10, 11 },
            m2x4 = { 5, 6, 7, 8, 9, 10, 11, 12 },
            m3x2 = { 5, 6, 9, 10, 13, 14 },
            m3x3 = { 5, 6, 7, 9, 10, 11, 13, 14, 15 },
            m3x4 = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 },
            m4x2 = { 5, 6, 9, 10, 13, 14, 17, 18 },
            m4x3 = { 5, 6, 7, 9, 10, 11, 13, 14, 15, 17, 18, 19 },
            m4x4 = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 }
        }
    )script";

    ghoul::Dictionary d;
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromString(script, d));
    REQUIRE(d.size() == 25);

    glm::vec2 vec2Value;
    bool success = d.getValue("n2", vec2Value);
    REQUIRE(success);
    success = d.getValue("num2", vec2Value);
    REQUIRE(success);
    success = d.getValue("xy", vec2Value);
    REQUIRE(success);
    success = d.getValue("rg", vec2Value);
    REQUIRE(success);
    success = d.getValue("st", vec2Value);
    REQUIRE(success);

    glm::vec3 vec3Value;
    success = d.getValue("n3", vec3Value);
    REQUIRE(success);
    success = d.getValue("num3", vec3Value);
    REQUIRE(success);
    success = d.getValue("xyz", vec3Value);
    REQUIRE(success);
    success = d.getValue("rgb", vec3Value);
    REQUIRE(success);
    success = d.getValue("stp", vec3Value);
    REQUIRE(success);
    success = d.getValue("mix", vec3Value);
    REQUIRE(success);

    glm::vec4 vec4Value;
    success = d.getValue("n4", vec4Value);
    REQUIRE(success);
    success = d.getValue("num4", vec4Value);
    REQUIRE(success);
    success = d.getValue("xyzw", vec4Value);
    REQUIRE(success);
    success = d.getValue("rgba", vec4Value);
    REQUIRE(success);
    success = d.getValue("stpq", vec4Value);
    REQUIRE(success);

    glm::mat2x2 mat2x2Value;
    success = d.getValue("m2x2", mat2x2Value);
    REQUIRE(success);

    glm::mat2x3 mat2x3Value;
    success = d.getValue("m2x3", mat2x3Value);
    REQUIRE(success);

    glm::mat2x4 mat2x4Value;
    success = d.getValue("m2x4", mat2x4Value);
    REQUIRE(success);

    glm::mat3x2 mat3x2Value;
    success = d.getValue("m3x2", mat3x2Value);
    REQUIRE(success);

    glm::mat3x3 mat3x3Value;
    success = d.getValue("m3x3", mat3x3Value);
    REQUIRE(success);

    glm::mat3x4 mat3x4Value;
    success = d.getValue("m3x4", mat3x4Value);
    REQUIRE(success);

    glm::mat4x2 mat4x2Value;
    success = d.getValue("m4x2", mat4x2Value);
    REQUIRE(success);

    glm::mat4x3 mat4x3Value;
    success = d.getValue("m4x3", mat4x3Value);
    REQUIRE(success);

    glm::mat4x4 mat4x4Value;
    success = d.getValue("m4x4", mat4x4Value);
    REQUIRE(success);
}
