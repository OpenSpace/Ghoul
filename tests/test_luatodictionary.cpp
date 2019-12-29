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

#if 0

#include <ghoul/glm.h>
#include <fstream>
#include <random>
#include <ghoul/misc/dictionary.h>
#include <ghoul/lua/lua_helper.h>

class LuaToDictionaryTest : public testing::Test {
protected:
    void SetUp() override {
        _d.clear();

        // This is not necessary to do every SetUp phase, but I don't know of a better
        // way to do it once per fixture ---abock
        _test0 = constants::TestDirectory + "/luatodictionary/test0.cfg";
        _test1 = constants::TestDirectory + "/luatodictionary/test1.cfg";
        _test2 = constants::TestDirectory + "/luatodictionary/test2.cfg";
        _test3 = constants::TestDirectory + "/luatodictionary/test3.cfg";
        _test4 = constants::TestDirectory + "/luatodictionary/test4.cfg";
        _test5 = constants::TestDirectory + "/luatodictionary/test5.cfg";
    }

    ghoul::Dictionary _d;

    std::string _test0;
    std::string _test1;
    std::string _test2;
    std::string _test3;
    std::string _test4;
    std::string _test5;
};


TEST_F(LuaToDictionaryTest, LoadTest1Cfg) {
    // test1.cfg
    // return {
    //    t = 1
    //}

    _d.clear();
    ASSERT_NO_THROW(ghoul::lua::loadDictionaryFromFile(_test1, _d));
    EXPECT_EQ(1, _d.size());

    double value;
    bool success = _d.getValue("t", value);
    ASSERT_EQ(true, success);
    EXPECT_EQ(1.0, value);
}

TEST_F(LuaToDictionaryTest, LoadTest2Cfg) {
    // test2.cfg
    //return{
    //    t = 2,
    //    s = { a = "a", b = "b", a1 = "1" }
    //}

    _d.clear();
    ASSERT_NO_THROW(ghoul::lua::loadDictionaryFromFile(_test2, _d));

    EXPECT_EQ(2, _d.size());

    double value;
    bool success = _d.getValue("t", value);
    ASSERT_EQ(true, success);
    EXPECT_EQ(2.0, value);

    ghoul::Dictionary dict;
    success = _d.getValue("s", dict);
    ASSERT_EQ(true, success);
    ASSERT_EQ(3, dict.size());

    std::string strValue;
    success = dict.getValue("a", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("a", strValue);

    success = dict.getValue("b", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("b", strValue);

    success = dict.getValue("a1", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("1", strValue);
}

TEST_F(LuaToDictionaryTest, LoadTest3Cfg) {
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

    _d.clear();
    ASSERT_NO_THROW(ghoul::lua::loadDictionaryFromFile(_test3, _d));

    EXPECT_EQ(2, _d.size());

    ghoul::Dictionary dict;
    bool success = _d.getValue("s", dict);
    ASSERT_EQ(true, success);
    ASSERT_EQ(3, dict.size());
    std::string strValue;
    success = dict.getValue("1", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("1", strValue);
    success = dict.getValue("2", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("2", strValue);

    ghoul::Dictionary dict2;
    success = dict.getValue("3", dict2);
    ASSERT_EQ(true, success);
    ASSERT_EQ(2, dict2.size());

    success = dict2.getValue("a", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("3a", strValue);
    success = dict2.getValue("b", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("3b", strValue);
}

TEST_F(LuaToDictionaryTest, LoadTest4Cfg) {
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
    _d.clear();
    ASSERT_NO_THROW(ghoul::lua::loadDictionaryFromFile(_test4, _d));

    ASSERT_EQ(25, _d.size());

    glm::vec2 vec2Value;
    bool success = _d.getValue("n2", vec2Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("num2", vec2Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("xy", vec2Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("rg", vec2Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("st", vec2Value);
    EXPECT_EQ(true, success);

    glm::vec3 vec3Value;
    success = _d.getValue("n3", vec3Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("num3", vec3Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("xyz", vec3Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("rgb", vec3Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("stp", vec3Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("mix", vec3Value);
    EXPECT_EQ(true, success);

    glm::vec4 vec4Value;
    success = _d.getValue("n4", vec4Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("num4", vec4Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("xyzw", vec4Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("rgba", vec4Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("stpq", vec4Value);
    EXPECT_EQ(true, success);

    glm::mat2x2 mat2x2Value;
    success = _d.getValue("m2x2", mat2x2Value);
    EXPECT_EQ(true, success);

    glm::mat2x3 mat2x3Value;
    success = _d.getValue("m2x3", mat2x3Value);
    EXPECT_EQ(true, success);

    glm::mat2x4 mat2x4Value;
    success = _d.getValue("m2x4", mat2x4Value);
    EXPECT_EQ(true, success);

    glm::mat3x2 mat3x2Value;
    success = _d.getValue("m3x2", mat3x2Value);
    EXPECT_EQ(true, success);

    glm::mat3x3 mat3x3Value;
    success = _d.getValue("m3x3", mat3x3Value);
    EXPECT_EQ(true, success);

    glm::mat3x4 mat3x4Value;
    success = _d.getValue("m3x4", mat3x4Value);
    EXPECT_EQ(true, success);

    glm::mat4x2 mat4x2Value;
    success = _d.getValue("m4x2", mat4x2Value);
    EXPECT_EQ(true, success);

    glm::mat4x3 mat4x3Value;
    success = _d.getValue("m4x3", mat4x3Value);
    EXPECT_EQ(true, success);

    glm::mat4x4 mat4x4Value;
    success = _d.getValue("m4x4", mat4x4Value);
    EXPECT_EQ(true, success);
}

TEST_F(LuaToDictionaryTest, LoadTest1CfgDirect) {
    const std::string script = R"script(
    return {
        t = 1
    }
    )script";

    _d.clear();
    ASSERT_NO_THROW(ghoul::lua::loadDictionaryFromString(script, _d));
    EXPECT_EQ(1, _d.size());

    double value;
    bool success = _d.getValue("t", value);
    ASSERT_EQ(true, success);
    EXPECT_EQ(1.0, value);
}

TEST_F(LuaToDictionaryTest, LoadTest2CfgDirect) {
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

    _d.clear();

    ASSERT_NO_THROW(ghoul::lua::loadDictionaryFromString(script, _d));
    EXPECT_EQ(2, _d.size());

    double value;
    bool success = _d.getValue("t", value);
    ASSERT_EQ(true, success);
    EXPECT_EQ(2.0, value);

    ghoul::Dictionary dict;
    success = _d.getValue("s", dict);
    ASSERT_EQ(true, success);
    ASSERT_EQ(3, dict.size());

    std::string strValue;
    success = dict.getValue("a", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("a", strValue);

    success = dict.getValue("b", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("b", strValue);

    success = dict.getValue("a1", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("1", strValue);
}

TEST_F(LuaToDictionaryTest, LoadTest3CfgDirect) {
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

    _d.clear();
    ASSERT_NO_THROW(ghoul::lua::loadDictionaryFromString(script, _d));
    EXPECT_EQ(2, _d.size());

    ghoul::Dictionary dict;
    bool success = _d.getValue("s", dict);
    ASSERT_EQ(true, success);
    ASSERT_EQ(3, dict.size());
    std::string strValue;
    success = dict.getValue("1", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("1", strValue);
    success = dict.getValue("2", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("2", strValue);

    ghoul::Dictionary dict2;
    success = dict.getValue("3", dict2);
    ASSERT_EQ(true, success);
    ASSERT_EQ(2, dict2.size());

    success = dict2.getValue("a", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("3a", strValue);
    success = dict2.getValue("b", strValue);
    ASSERT_EQ(true, success);
    EXPECT_EQ("3b", strValue);
}

TEST_F(LuaToDictionaryTest, LoadTest4CfgDirect) {
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

    _d.clear();
    ASSERT_NO_THROW(ghoul::lua::loadDictionaryFromString(script, _d));
    ASSERT_EQ(25, _d.size());

    glm::vec2 vec2Value;
    bool success = _d.getValue("n2", vec2Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("num2", vec2Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("xy", vec2Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("rg", vec2Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("st", vec2Value);
    EXPECT_EQ(true, success);

    glm::vec3 vec3Value;
    success = _d.getValue("n3", vec3Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("num3", vec3Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("xyz", vec3Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("rgb", vec3Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("stp", vec3Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("mix", vec3Value);
    EXPECT_EQ(true, success);

    glm::vec4 vec4Value;
    success = _d.getValue("n4", vec4Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("num4", vec4Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("xyzw", vec4Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("rgba", vec4Value);
    EXPECT_EQ(true, success);
    success = _d.getValue("stpq", vec4Value);
    EXPECT_EQ(true, success);

    glm::mat2x2 mat2x2Value;
    success = _d.getValue("m2x2", mat2x2Value);
    EXPECT_EQ(true, success);

    glm::mat2x3 mat2x3Value;
    success = _d.getValue("m2x3", mat2x3Value);
    EXPECT_EQ(true, success);

    glm::mat2x4 mat2x4Value;
    success = _d.getValue("m2x4", mat2x4Value);
    EXPECT_EQ(true, success);

    glm::mat3x2 mat3x2Value;
    success = _d.getValue("m3x2", mat3x2Value);
    EXPECT_EQ(true, success);

    glm::mat3x3 mat3x3Value;
    success = _d.getValue("m3x3", mat3x3Value);
    EXPECT_EQ(true, success);

    glm::mat3x4 mat3x4Value;
    success = _d.getValue("m3x4", mat3x4Value);
    EXPECT_EQ(true, success);

    glm::mat4x2 mat4x2Value;
    success = _d.getValue("m4x2", mat4x2Value);
    EXPECT_EQ(true, success);

    glm::mat4x3 mat4x3Value;
    success = _d.getValue("m4x3", mat4x3Value);
    EXPECT_EQ(true, success);

    glm::mat4x4 mat4x4Value;
    success = _d.getValue("m4x4", mat4x4Value);
    EXPECT_EQ(true, success);
}

#endif