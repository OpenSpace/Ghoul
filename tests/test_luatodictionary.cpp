/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2020                                                               *
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

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/lua/lua_helper.h>
#include <ghoul/lua/luastate.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/glm.h>
#include <fstream>
#include <sstream>
#include <iostream>

TEST_CASE("LuaToDictionary: Nested Tables", "[luatodictionary]") {
    constexpr const char* TestString = R"(
        glob = {
            A = {
                B = {
                    C = {
                        D = {
                            E = { 
                                F = { "127.0.0.1", "localhost" },
                                G = {}
                            }
                        }
                    }
                }
            }
        }
)";

    ghoul::lua::LuaState state(ghoul::lua::LuaState::IncludeStandardLibrary::No);
    ghoul::lua::runScript(state, TestString);
    //ghoul::lua::runScriptFile(state, "C:/Users/alebo68/Desktop/test.lua");

    lua_getglobal(state, "glob");

    ghoul::Dictionary dict;
    ghoul::lua::luaDictionaryFromState(state, dict);

    REQUIRE(dict.hasKeyAndValue<ghoul::Dictionary>("A"));
    ghoul::Dictionary a = dict.value<ghoul::Dictionary>("A");

    REQUIRE(a.hasKeyAndValue<ghoul::Dictionary>("B"));
    ghoul::Dictionary b = a.value<ghoul::Dictionary>("B");

    REQUIRE(b.hasKeyAndValue<ghoul::Dictionary>("C"));
    ghoul::Dictionary c = b.value<ghoul::Dictionary>("C");

    REQUIRE(c.hasKeyAndValue<ghoul::Dictionary>("D"));
    ghoul::Dictionary d = c.value<ghoul::Dictionary>("D");

    REQUIRE(d.hasKeyAndValue<ghoul::Dictionary>("E"));
    ghoul::Dictionary e = d.value<ghoul::Dictionary>("E");

    REQUIRE(e.hasKeyAndValue<ghoul::Dictionary>("F"));
    ghoul::Dictionary f = e.value<ghoul::Dictionary>("F");

    REQUIRE(f.hasKeyAndValue<std::string>("1"));
    REQUIRE(f.hasKeyAndValue<std::string>("2"));
}

TEST_CASE("LuaToDictionary: Nested Tables 2", "[luatodictionary]") {
    constexpr const char* TestString = R"(
        ModuleConfigurations = {
            Server = {
                Interfaces = {
                    {
                        RequirePasswordAddresses = {}
                    },
                    {
                        RequirePasswordAddresses = {}
                    }
                }
            }
        }
)";

    ghoul::lua::LuaState state(ghoul::lua::LuaState::IncludeStandardLibrary::No);
    ghoul::lua::runScript(state, TestString);

    lua_getglobal(state, "ModuleConfigurations");
    ghoul::Dictionary d = ghoul::lua::value<ghoul::Dictionary>(state);
    REQUIRE(d.hasKeyAndValue<ghoul::Dictionary>("Server"));
}


// @TODO (abock, 2020-01-06) None of these tests really work anymore and there is an open
// issue to rewrite the Dictionary class, so there is no much reason to make these work
// again first

#if 0

namespace {
    // A non-existing configuration file
    const std::string _configuration0 = "${UNIT_TEST}/luatodictionary/test0.cfg";

    // The configuration1 test configuration has one key "t" = 1
    const std::string _configuration1 = "${UNIT_TEST}/luatodictionary/test1.cfg";

    // The configuration1 test configuration has two keys "t" and "s"
    const std::string _configuration2 = "${UNIT_TEST}/luatodictionary/test2.cfg";

    // More complicated configuration file with nested tables
    const std::string _configuration3 = "${UNIT_TEST}/luatodictionary/test3.cfg";

    // Deeply nested configuration file with 12 level
    const std::string _configuration4 = "${UNIT_TEST}/luatodictionary/test4.cfg";

    // Testfile with glm::vecX, glm::matX
    const std::string _configuration5 = "${UNIT_TEST}/luatodictionary/test5.cfg";
} // namespace

TEST_CASE("LuaToDictionary: Reinit", "[luatodictionary]") {
    ghoul::Dictionary m;

    m.setValue("t", int(2));
    m.clear();
    const bool success = m.hasKey("t");
    REQUIRE_FALSE(success);
}

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
    REQUIRE(d.getValue("t", value));
    REQUIRE(value == 2.0);

    ghoul::Dictionary dict;
    REQUIRE(d.getValue("s", dict));
    REQUIRE(dict.size() == 3);

    std::string strValue;
    REQUIRE(dict.getValue("a", strValue));
    REQUIRE(strValue == "a");

    REQUIRE(dict.getValue("b", strValue));
    REQUIRE(strValue == "b");

    REQUIRE(dict.getValue("a1", strValue));
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
    REQUIRE(d.getValue("s", dict));
    REQUIRE(dict.size() == 3);
    std::string strValue;
    REQUIRE(dict.getValue("1", strValue));
    REQUIRE(strValue == "1");
    REQUIRE(dict.getValue("2", strValue));
    REQUIRE(strValue == "2");

    ghoul::Dictionary dict2;
    REQUIRE(dict.getValue("3", dict2));
    REQUIRE(dict2.size() == 2);

    REQUIRE(dict2.getValue("a", strValue));
    REQUIRE(strValue == "3a");
    REQUIRE(dict2.getValue("b", strValue));
    REQUIRE(strValue == "3b");
}

TEST_CASE("LuaToDictionary: Load Test5 cfg", "[luatodictionary]") {
    // test5.cfg
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
    std::string test4 = absPath("${UNIT_TEST}/luatodictionary/test5.cfg");
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(test4, d));

    REQUIRE(d.size() == 25);

    glm::vec2 vec2Value;
    REQUIRE(d.getValue("n2", vec2Value));
    REQUIRE(d.getValue("num2", vec2Value));
    REQUIRE(d.getValue("xy", vec2Value));
    REQUIRE(d.getValue("rg", vec2Value));
    REQUIRE(d.getValue("st", vec2Value));

    glm::vec3 vec3Value;
    REQUIRE(d.getValue("n3", vec3Value));
    REQUIRE(d.getValue("num3", vec3Value));
    REQUIRE(d.getValue("xyz", vec3Value));
    REQUIRE(d.getValue("rgb", vec3Value));
    REQUIRE(d.getValue("stp", vec3Value));
    REQUIRE(d.getValue("mix", vec3Value));

    glm::vec4 vec4Value;
    REQUIRE(d.getValue("n4", vec4Value));
    REQUIRE(d.getValue("num4", vec4Value));
    REQUIRE(d.getValue("xyzw", vec4Value));
    REQUIRE(d.getValue("rgba", vec4Value));
    REQUIRE(d.getValue("stpq", vec4Value));

    glm::mat2x2 mat2x2Value;
    REQUIRE(d.getValue("m2x2", mat2x2Value));

    glm::mat2x3 mat2x3Value;
    REQUIRE(d.getValue("m2x3", mat2x3Value));

    glm::mat2x4 mat2x4Value;
    REQUIRE(d.getValue("m2x4", mat2x4Value));

    glm::mat3x2 mat3x2Value;
    REQUIRE(d.getValue("m3x2", mat3x2Value));

    glm::mat3x3 mat3x3Value;
    REQUIRE(d.getValue("m3x3", mat3x3Value));

    glm::mat3x4 mat3x4Value;
    REQUIRE(d.getValue("m3x4", mat3x4Value));

    glm::mat4x2 mat4x2Value;
    REQUIRE(d.getValue("m4x2", mat4x2Value));

    glm::mat4x3 mat4x3Value;
    REQUIRE(d.getValue("m4x3", mat4x3Value));

    glm::mat4x4 mat4x4Value;
    REQUIRE(d.getValue("m4x4", mat4x4Value));
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
    REQUIRE(d.getValue("t", value));
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
    REQUIRE(d.getValue("t", value));
    REQUIRE(value == 2.0);

    ghoul::Dictionary dict;
    REQUIRE(d.getValue("s", dict));
    REQUIRE(dict.size() == 3);

    std::string strValue;
    REQUIRE(dict.getValue("a", strValue));
    REQUIRE(strValue == "a");

    REQUIRE(dict.getValue("b", strValue));
    REQUIRE(strValue == "b");

    REQUIRE(dict.getValue("a1", strValue));
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
    REQUIRE(d.getValue("s", dict));
    REQUIRE(dict.size() == 3);
    std::string strValue;
    REQUIRE(dict.getValue("1", strValue));
    REQUIRE(strValue == "1");
    REQUIRE(dict.getValue("2", strValue));
    REQUIRE(strValue == "2");

    ghoul::Dictionary dict2;
    REQUIRE(dict.getValue("3", dict2));
    REQUIRE(dict2.size() == 2);

    REQUIRE(dict2.getValue("a", strValue));
    REQUIRE(strValue == "3a");
    REQUIRE(dict2.getValue("b", strValue));
    REQUIRE(strValue == "3b");
}

TEST_CASE("LuaToDictionary: Load Test5 cfg Direct", "[luatodictionary]") {
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
    REQUIRE(d.getValue("n2", vec2Value));
    REQUIRE(d.getValue("num2", vec2Value));
    REQUIRE(d.getValue("xy", vec2Value));
    REQUIRE(d.getValue("rg", vec2Value));
    REQUIRE(d.getValue("st", vec2Value));

    glm::vec3 vec3Value;
    REQUIRE(d.getValue("n3", vec3Value));
    REQUIRE(d.getValue("num3", vec3Value));
    REQUIRE(d.getValue("xyz", vec3Value));
    REQUIRE(d.getValue("rgb", vec3Value));
    REQUIRE(d.getValue("stp", vec3Value));
    REQUIRE(d.getValue("mix", vec3Value));

    glm::vec4 vec4Value;
    REQUIRE(d.getValue("n4", vec4Value));
    REQUIRE(d.getValue("num4", vec4Value));
    REQUIRE(d.getValue("xyzw", vec4Value));
    REQUIRE(d.getValue("rgba", vec4Value));
    REQUIRE(d.getValue("stpq", vec4Value));

    glm::mat2x2 mat2x2Value;
    REQUIRE(d.getValue("m2x2", mat2x2Value));

    glm::mat2x3 mat2x3Value;
    REQUIRE(d.getValue("m2x3", mat2x3Value));

    glm::mat2x4 mat2x4Value;
    REQUIRE(d.getValue("m2x4", mat2x4Value));

    glm::mat3x2 mat3x2Value;
    REQUIRE(d.getValue("m3x2", mat3x2Value));

    glm::mat3x3 mat3x3Value;
    REQUIRE(d.getValue("m3x3", mat3x3Value));

    glm::mat3x4 mat3x4Value;
    REQUIRE(d.getValue("m3x4", mat3x4Value));

    glm::mat4x2 mat4x2Value;
    REQUIRE(d.getValue("m4x2", mat4x2Value));

    glm::mat4x3 mat4x3Value;
    REQUIRE(d.getValue("m4x3", mat4x3Value));

    glm::mat4x4 mat4x4Value;
    REQUIRE(d.getValue("m4x4", mat4x4Value));
}

TEST_CASE("LuaToDictionary: Load Configuration", "[luatodictionary]") {
    ghoul::Dictionary m;

    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration1), m));
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration2), m));
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration3), m));
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration4), m));
}

TEST_CASE("LuaToDictionary: Keys Function", "[luatodictionary]") {
    ghoul::Dictionary m;

    // The empty configuration should not have any keys
    SECTION("empty") {
        REQUIRE(m.keys().size() == 0);
    }

    SECTION("config1") {
        REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration1), m));
        REQUIRE(m.keys().size() == 1);
    }

    SECTION("config3") {
        REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration3), m));
        REQUIRE(m.keys().size() == 2);
        REQUIRE(m.keys("s").size() == 3);
        REQUIRE(m.keys("s.3").size() == 2);
    }

    SECTION("config4") {
        REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration4), m));

        const char* keys[] = {
            "a", "a.a", "a.a.a", "a.a.a.a", "a.a.a.a.a", "a.a.a.a.a.a", "a.a.a.a.a.a.a",
            "a.a.a.a.a.a.a.a", "a.a.a.a.a.a.a.a.a", "a.a.a.a.a.a.a.a.a.a",
            "a.a.a.a.a.a.a.a.a.a.a", "a.a.a.a.a.a.a.a.a.a.a.a"
        };

        for (int i = 0; i < 12; ++i) {
            REQUIRE(m.keys(keys[i]).size() == 2);
        }

        for (int i = 0; i < 12; ++i) {
            REQUIRE(m.hasKey(keys[i]));
        }

        constexpr const char* keysB[] = {
            "b", "b.b", "b.b.b", "b.b.b.b", "b.b.b.b.b", "b.b.b.b.b.b", "b.b.b.b.b.b.b",
            "b.b.b.b.b.b.b.b", "b.b.b.b.b.b.b.b.b", "b.b.b.b.b.b.b.b.b.b",
            "b.b.b.b.b.b.b.b.b.b.b", "b.b.b.b.b.b.b.b.b.b.b.b"
        };
        m.setValue(keysB[11], int(0), ghoul::Dictionary::CreateIntermediate::Yes);
        for (int i = 0; i < 12; ++i) {
            REQUIRE(m.hasKey(keysB[i]));
        }
    }
}

TEST_CASE("LuaToDictionary: HasKey Subtable", "[luatodictionary]") {
    ghoul::Dictionary m;
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration1), m));
    SECTION("t") {
        REQUIRE(m.hasKey("t"));
    }

    SECTION("t.s") {
        REQUIRE_FALSE(m.hasKey("t.s"));
    }

    SECTION("s") {
        REQUIRE_FALSE(m.hasKey("s"));
    }

    SECTION("s.x") {
        REQUIRE_FALSE(m.hasKey("s.x"));
    }
}

TEST_CASE("LuaToDictionary: HasKey Types", "[luatodictionary]") {
    ghoul::Dictionary m;

    m.setValue("t", ghoul::Dictionary());
    m.setValue("t.bool", bool(1));
    m.setValue("t.char", char(1));
    m.setValue("t.unsignedchar", static_cast<unsigned char>(1));
    m.setValue("t.signedchar", static_cast<signed char>(1));
    m.setValue("t.wchar", wchar_t(1));
    m.setValue("t.short", short(1));
    m.setValue("t.unsignedshort", static_cast<unsigned short>(1));
    m.setValue("t.int", int(1));
    m.setValue("t.unsignedint", static_cast<unsigned int>(1));
    m.setValue("t.long", long(1));
    m.setValue("t.unsignedlong", static_cast<unsigned long>(1));
    m.setValue("t.longlong", static_cast<long long>(1));
    m.setValue("t.unsignedlonglong", static_cast<unsigned long long>(1));
    m.setValue("t.float", float(1));
    m.setValue("t.double", double(1));
    m.setValue("t.longdouble", static_cast<long double>(1));
    m.setValue("t.string", "1");

    SECTION("t.bool") {
        REQUIRE(m.hasKey("t.bool"));
    }
    SECTION("t.char") {
        REQUIRE(m.hasKey("t.char"));
    }
    SECTION("t.unsignedchar") {
        REQUIRE(m.hasKey("t.unsignedchar"));
    }
    SECTION("t.signedchar") {
        REQUIRE(m.hasKey("t.signedchar"));
    }
    SECTION("t.wchar") {
        REQUIRE(m.hasKey("t.wchar"));
    }
    SECTION("t.short") {
        REQUIRE(m.hasKey("t.short"));
    }
    SECTION("t.unsignedshort") {
        REQUIRE(m.hasKey("t.unsignedshort"));
    }
    SECTION("t.int") {
        REQUIRE(m.hasKey("t.int"));
    }
    SECTION("t.unsignedint") {
        REQUIRE(m.hasKey("t.unsignedint"));
    }
    SECTION("t.long") {
        REQUIRE(m.hasKey("t.long"));
    }
    SECTION("t.unsignedlong") {
        REQUIRE(m.hasKey("t.unsignedlong"));
    }
    SECTION("t.longlong") {
        REQUIRE(m.hasKey("t.longlong"));
    }
    SECTION("t.unsignedlonglong") {
        REQUIRE(m.hasKey("t.unsignedlonglong"));
    }
    SECTION("t.float") {
        REQUIRE(m.hasKey("t.float"));
    }
    SECTION("t.double") {
        REQUIRE(m.hasKey("t.double"));
    }
    SECTION("t.longdouble") {
        REQUIRE(m.hasKey("t.longdouble"));
    }
    SECTION("t.string") {
        REQUIRE(m.hasKey("t.string"));
    }
}

TEST_CASE("LuaToDictionary: GetValue Function", "[luatodictionary]") {
    ghoul::Dictionary m;

    SECTION("empty") {
        std::string test;
        REQUIRE_FALSE(m.getValue("key", test));
    }

    SECTION("missing key") {
        std::string test;
        REQUIRE_FALSE(m.getValue("key.key", test));
    }

    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration1), m));
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration3), m));

    SECTION("t") {
        double testInt;
        REQUIRE(m.getValue("t", testInt));
        REQUIRE(testInt == 1.0);
    }

    SECTION("s.a") {
        std::string test;
        REQUIRE_FALSE(m.getValue("s.a", test));
    }

    SECTION("s.1") {
        std::string test;
        REQUIRE(m.getValue("s.1", test));
    }

    SECTION("s.1.a") {
        std::string test;
        REQUIRE_FALSE(m.getValue("s.1.a", test));
    }

    SECTION("s.3.a") {
        std::string test;
        REQUIRE(m.getValue("s.3.a", test));
    }

    SECTION("vector") {
        std::vector<int> testVec;
        REQUIRE_FALSE(m.getValue("key", testVec));
    }
}

TEST_CASE("LuaToDictionary: GetValue Correctness", "[luatodictionary]") {
    ghoul::Dictionary m;

    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration1), m));

    SECTION("bool") {
        bool value = false;
        REQUIRE(m.getValue("t", value));
        REQUIRE(value);
    }

    SECTION("char") {
        char value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("signed char") {
        signed char value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned char") {
        unsigned char value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("wchar_t") {
        wchar_t value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("short") {
        short value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned short") {
        unsigned short value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("int") {
        int value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned int") {
        unsigned int value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("long") {
        long value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned long") {
        unsigned long value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("long long") {
        long long value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned long long") {
        unsigned long long value(0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1);
    }

    SECTION("float") {
        float value(0.f);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1.f);
    }

    SECTION("double") {
        double value(0.0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1.0);
    }

    SECTION("long double") {
        long double value(0.0);
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == 1.0);
    }

    SECTION("string") {
        std::string value;
        REQUIRE(m.getValue("t", value));
        REQUIRE(value == "1");
    }
}

TEST_CASE("LuaToDictionary: SetValue Recursive", "[luatodictionary]") {
    ghoul::Dictionary m;

    m.setValue("t.a.b.c", 1);
    REQUIRE(m.hasKey("t"));
    REQUIRE(m.hasKey("t.a"));
    REQUIRE(m.hasKey("t.a.b"));
    REQUIRE(m.hasKey("t.a.b.c"));
}

TEST_CASE("LuaToDictionary: SetValue Correctness", "[luatodictionary]") {
    ghoul::Dictionary m;

    m.setValue("t.bool", bool(1));
    m.setValue("t.char", char(1));
    m.setValue("t.unsignedchar", static_cast<unsigned char>(1));
    m.setValue("t.signedchar", static_cast<signed char>(1));
    m.setValue("t.wchar", wchar_t(1));
    m.setValue("t.short", short(1));
    m.setValue("t.unsignedshort", static_cast<unsigned short>(1));
    m.setValue("t.int", int(1));
    m.setValue("t.unsignedint", static_cast<unsigned int>(1));
    m.setValue("t.long", long(1));
    m.setValue("t.unsignedlong", static_cast<unsigned long>(1));
    m.setValue("t.longlong", static_cast<long long>(1));
    m.setValue("t.unsignedlonglong", static_cast<unsigned long long>(1));
    m.setValue("t.float", float(1));
    m.setValue("t.double", double(1));
    m.setValue("t.longdouble", static_cast<long double>(1));
    m.setValue("t.string", "1");

    SECTION("bool") {
        bool value = false;
        REQUIRE(m.getValue("t.bool", value));
        REQUIRE(value);
    }

    SECTION("char") {
        char value(0);
        REQUIRE(m.getValue("t.char", value));
        REQUIRE(value == 1);
    }

    SECTION("signed char") {
        signed char value(0);
        REQUIRE(m.getValue("t.signedchar", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned char") {
        unsigned char value(0);
        REQUIRE(m.getValue("t.unsignedchar", value));
        REQUIRE(value == 1);
    }

    SECTION("wchar_t") {
        wchar_t value(0);
        REQUIRE(m.getValue("t.wchar", value));
        REQUIRE(value == 1);
    }

    SECTION("short") {
        short value(0);
        REQUIRE(m.getValue("t.short", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned short") {
        unsigned short value(0);
        REQUIRE(m.getValue("t.unsignedshort", value));
        REQUIRE(value == 1);
    }

    SECTION("int") {
        int value(0);
        REQUIRE(m.getValue("t.int", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned int") {
        unsigned int value(0);
        REQUIRE(m.getValue("t.unsignedint", value));
        REQUIRE(value == 1);
    }

    SECTION("long") {
        long value(0);
        REQUIRE(m.getValue("t.long", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned long") {
        unsigned long value(0);
        REQUIRE(m.getValue("t.unsignedlong", value));
        REQUIRE(value == 1);
    }

    SECTION("long long") {
        long long value(0);
        REQUIRE(m.getValue("t.longlong", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned long long") {
        unsigned long long value(0);
        REQUIRE(m.getValue("t.unsignedlonglong", value));
        REQUIRE(value == 1);
    }

    SECTION("float") {
        float value(0.f);
        REQUIRE(m.getValue("t.float", value));
        REQUIRE(value == 1.f);
    }

    SECTION("double") {
        double value(0.0);
        REQUIRE(m.getValue("t.double", value));
        REQUIRE(value == 1.0);
    }

    SECTION("long double") {
        long double value(0.0);
        REQUIRE(m.getValue("t.longdouble", value));
        REQUIRE(value == 1.0);
    }

    SECTION("string") {
        std::string value;
        REQUIRE(m.getValue("t.string", value));
        REQUIRE(value == "1");
    }
}

TEST_CASE("LuaToDictionary: SetValue Overrides Configuration", "[luatodictionary]") {
    ghoul::Dictionary m;

    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration1), m));

    int v1 = 0;
    REQUIRE(m.getValue<int>("t", v1));
    REQUIRE(v1 == 1);

    m.setValue("t", int(2));
    int v2 = 0;
    REQUIRE(m.getValue<int>("t", v2));
    REQUIRE(v2 == 2);
}

TEST_CASE("LuaToDictionary: GetValue Conversions", "[luatodictionary]") {
    ghoul::Dictionary m;

    // converting from 1 -> all types is done in GetValueCorrectness
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration2), m));

    SECTION("bool") {
        bool value = false;
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value);
    }

    SECTION("char") {
        char value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("signed char") {
        signed char value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned char") {
        unsigned char value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("wchar_t") {
        wchar_t value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("short") {
        short value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned short") {
        unsigned short value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("int") {
        int value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned int") {
        unsigned int value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("long") {
        long value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned long") {
        unsigned long value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("long long") {
        long long value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("unsigned long long") {
        unsigned long long value(0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1);
    }

    SECTION("float") {
        float value(0.f);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1.f);
    }

    SECTION("double") {
        double value(0.0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1.0);
    }

    SECTION("long double") {
        long double value(0.0);
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == 1.0);
    }

    SECTION("string") {
        std::string value;
        REQUIRE(m.getValue("s.a1", value));
        REQUIRE(value == "1");
    }
}

TEST_CASE("LuaToDictionary: String Key vs Int Key", "[luatodictionary]") {
    ghoul::Dictionary m;

    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration3), m));

    int v1 = 0;
    REQUIRE(m.getValue("tt[\"1\"]", v1));
    REQUIRE(v1 == 2);

    int v2 = 0;
    REQUIRE(m.getValue("tt[1]", v2));
    REQUIRE(v2 == 1);
}

TEST_CASE("LuaToDictionary: Invalid Key Access Invariant", "[luatodictionary]")
{
    ghoul::Dictionary m;

    // Accessing an invalid key should not change the tested argument
    std::mt19937 rd;
    {
        std::uniform_int_distribution<int> dist;
        for (int i = 0; i < 10; ++i) {
            const int testValue = dist(rd);
            int test = testValue;
            REQUIRE_FALSE(m.getValue("key", test));
            REQUIRE(testValue == test);
        }
    }

    {
        std::uniform_real_distribution<float> dist;
        for (int i = 0; i < 10; ++i) {
            const float testValue = dist(rd);
            float test = testValue;
            REQUIRE_FALSE(m.getValue("key", test));
            REQUIRE(testValue == Approx(test));
        }
    }
}

TEST_CASE("LuaToDictionary: HasKey Function", "[luatodictionary]") {
    ghoul::Dictionary m;

    SECTION("empty") {
        REQUIRE_FALSE(m.hasKey("key"));
    }

    SECTION("test1 (t)") {
        REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration1), m));
        REQUIRE(m.hasKey("t"));
    }

    SECTION("test1 (s)") {
        REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration1), m));
        REQUIRE_FALSE(m.hasKey("s"));
    }

    SECTION("test1+test2 (s.a") {
        REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration1), m));
        REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration2), m));
        REQUIRE(m.hasKey("s.a"));
    }

    SECTION("test1+test2 (s.a") {
        REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration1), m));
        REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration2), m));
        REQUIRE_FALSE(m.hasKey("s.c"));
    }
}

TEST_CASE("LuaToDictionary: Multiple Key Load Overwrite", "[luatodictionary]") {
    ghoul::Dictionary m;

    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration1), m));
    int value1 = 0;
    REQUIRE(m.getValue("t", value1));
    REQUIRE(value1 == 1);

    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration2), m));

    // configuration2 should overwrite the value t in configuration1
    int value2 = 0;
    REQUIRE(m.getValue("t", value2));
    REQUIRE(value2 == 2);
}

TEST_CASE("LuaToDictionary: Vector Classes Get", "[luatodictionary]") {
    ghoul::Dictionary m;

    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration5), m));

    SECTION("glm::vec2") {
        glm::vec2 value = glm::vec2(0.f);
        SECTION("n2") {
            REQUIRE(m.getValue("n2", value));
            REQUIRE(value == glm::vec2(5.f, 6.f));
        }
        SECTION("num2") {
            REQUIRE(m.getValue("num2", value));
            REQUIRE(value == glm::vec2(5.f, 6.f));
        }
        SECTION("xy") {
            REQUIRE(m.getValue("xy", value));
            REQUIRE(value == glm::vec2(5.f, 6.f));
        }
        SECTION("rg") {
            REQUIRE(m.getValue("rg", value));
            REQUIRE(value == glm::vec2(5.f, 6.f));
        }
        SECTION("st") {
            REQUIRE(m.getValue("st", value));
            REQUIRE(value == glm::vec2(5.f, 6.f));
        }
    }

    SECTION("glm::dvec2") {
        glm::dvec2 value = glm::dvec2(0.0);
        SECTION("n2") {
            REQUIRE(m.getValue("n2", value));
            REQUIRE(value == glm::dvec2(5.0, 6.0));
        }
        SECTION("num2") {
            REQUIRE(m.getValue("num2", value));
            REQUIRE(value == glm::dvec2(5.0, 6.0));
        }
        SECTION("xy") {
            REQUIRE(m.getValue("xy", value));
            REQUIRE(value == glm::dvec2(5.0, 6.0));
        }
        SECTION("rg") {
            REQUIRE(m.getValue("rg", value));
            REQUIRE(value == glm::dvec2(5.0, 6.0));
        }
        SECTION("st") {
            REQUIRE(m.getValue("st", value));
            REQUIRE(value == glm::dvec2(5.0, 6.0));
        }
    }

    SECTION("glm::ivec2") {
        glm::ivec2 value = glm::ivec2(0);
        SECTION("n2") {
            REQUIRE(m.getValue("n2", value));
            REQUIRE(value == glm::ivec2(5, 6));
        }
        SECTION("num2") {
            REQUIRE(m.getValue("num2", value));
            REQUIRE(value == glm::ivec2(5, 6));
        }
        SECTION("xy") {
            REQUIRE(m.getValue("xy", value));
            REQUIRE(value == glm::ivec2(5, 6));
        }
        SECTION("rg") {
            REQUIRE(m.getValue("rg", value));
            REQUIRE(value == glm::ivec2(5, 6));
        }
        SECTION("st") {
            REQUIRE(m.getValue("st", value));
            REQUIRE(value == glm::ivec2(5, 6));
        }
    }

    SECTION("glm::uvec2") {
        glm::uvec2 value = glm::uvec2(0);
        SECTION("n2") {
            REQUIRE(m.getValue("n2", value));
            REQUIRE(value == glm::uvec2(5, 6));
        }
        SECTION("num2") {
            REQUIRE(m.getValue("num2", value));
            REQUIRE(value == glm::uvec2(5, 6));
        }
        SECTION("xy") {
            REQUIRE(m.getValue("xy", value));
            REQUIRE(value == glm::uvec2(5, 6));
        }
        SECTION("rg") {
            REQUIRE(m.getValue("rg", value));
            REQUIRE(value == glm::uvec2(5, 6));
        }
        SECTION("st") {
            REQUIRE(m.getValue("st", value));
            REQUIRE(value == glm::uvec2(5, 6));
        }
    }

    SECTION("glm::bvec2") {
        glm::bvec2 value = glm::bvec2(false);
        SECTION("n2") {
            REQUIRE(m.getValue("n2", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
        }
        SECTION("num2") {
            REQUIRE(m.getValue("num2", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
        }
        SECTION("xy") {
            REQUIRE(m.getValue("xy", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
        }
        SECTION("rg") {
            REQUIRE(m.getValue("rg", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
        }
        SECTION("st") {
            REQUIRE(m.getValue("st", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
        }
    }

    SECTION("glm::vec3") {
        glm::vec3 value = glm::vec3(0.f);
        SECTION("n3") {
            REQUIRE(m.getValue("n3", value));
            REQUIRE(value == glm::vec3(5.f, 6.f, 7.f));
        }
        SECTION("num3") {
            REQUIRE(m.getValue("num3", value));
            REQUIRE(value == glm::vec3(5.f, 6.f, 7.f));
        }
        SECTION("xyz") {
            REQUIRE(m.getValue("xyz", value));
            REQUIRE(value == glm::vec3(5.f, 6.f, 7.f));
        }
        SECTION("rgb") {
            REQUIRE(m.getValue("rgb", value));
            REQUIRE(value == glm::vec3(5.f, 6.f, 7.f));
        }
        SECTION("stp") {
            REQUIRE(m.getValue("stp", value));
            REQUIRE(value == glm::vec3(5.f, 6.f, 7.f));
        }
    }

    SECTION("glm::dvec3") {
        glm::dvec3 value = glm::dvec3(0.0);
        SECTION("n3") {
            REQUIRE(m.getValue("n3", value));
            REQUIRE(value == glm::dvec3(5.0, 6.0, 7.0));
        }
        SECTION("num3") {
            REQUIRE(m.getValue("num3", value));
            REQUIRE(value == glm::dvec3(5.0, 6.0, 7.0));
        }
        SECTION("xyz") {
            REQUIRE(m.getValue("xyz", value));
            REQUIRE(value == glm::dvec3(5.0, 6.0, 7.0));
        }
        SECTION("rgb") {
            REQUIRE(m.getValue("rgb", value));
            REQUIRE(value == glm::dvec3(5.0, 6.0, 7.0));
        }
        SECTION("stp") {
            REQUIRE(m.getValue("stp", value));
            REQUIRE(value == glm::dvec3(5.0, 6.0, 7.0));
        }
    }

    SECTION("glm::ivec3") {
        glm::ivec3 value = glm::ivec3(0);
        SECTION("n3") {
            REQUIRE(m.getValue("n3", value));
            REQUIRE(value == glm::ivec3(5, 6, 7));
        }
        SECTION("num3") {
            REQUIRE(m.getValue("num3", value));
            REQUIRE(value == glm::ivec3(5, 6, 7));
        }
        SECTION("xyz") {
            REQUIRE(m.getValue("xyz", value));
            REQUIRE(value == glm::ivec3(5, 6, 7));
        }
        SECTION("rgb") {
            REQUIRE(m.getValue("rgb", value));
            REQUIRE(value == glm::ivec3(5, 6, 7));
        }
        SECTION("stp") {
            REQUIRE(m.getValue("stp", value));
            REQUIRE(value == glm::ivec3(5, 6, 7));
        }
    }

    SECTION("glm::uvec3") {
        glm::uvec3 value = glm::uvec3(0);
        SECTION("n3") {
            REQUIRE(m.getValue("n3", value));
            REQUIRE(value == glm::uvec3(5, 6, 7));
        }
        SECTION("num3") {
            REQUIRE(m.getValue("num3", value));
            REQUIRE(value == glm::uvec3(5, 6, 7));
        }
        SECTION("xyz") {
            REQUIRE(m.getValue("xyz", value));
            REQUIRE(value == glm::uvec3(5, 6, 7));
        }
        SECTION("rgb") {
            REQUIRE(m.getValue("rgb", value));
            REQUIRE(value == glm::uvec3(5, 6, 7));
        }
        SECTION("stp") {
            REQUIRE(m.getValue("stp", value));
            REQUIRE(value == glm::uvec3(5, 6, 7));
        }
    }

    SECTION("glm::bvec3") {
        glm::bvec3 value = glm::bvec3(false);
        SECTION("n3") {
            REQUIRE(m.getValue("n3", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
            REQUIRE(value.z);
        }
        SECTION("num3") {
            REQUIRE(m.getValue("num3", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
            REQUIRE(value.z);
        }
        SECTION("xyz") {
            REQUIRE(m.getValue("xyz", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
            REQUIRE(value.z);
        }
        SECTION("rgb") {
            REQUIRE(m.getValue("rgb", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
            REQUIRE(value.z);
        }
        SECTION("stp") {
            REQUIRE(m.getValue("stp", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
            REQUIRE(value.z);
        }
    }

    SECTION("glm::vec4") {
        glm::vec4 value = glm::vec4(0.f);
        SECTION("n4") {
            REQUIRE(m.getValue("n4", value));
            REQUIRE(value == glm::vec4(5.f, 6.f, 7.f, 8.f));
        }
        SECTION("num4") {
            REQUIRE(m.getValue("num4", value));
            REQUIRE(value == glm::vec4(5.f, 6.f, 7.f, 8.f));
        }
        SECTION("xyzw") {
            REQUIRE(m.getValue("xyzw", value));
            REQUIRE(value == glm::vec4(5.f, 6.f, 7.f, 8.f));
        }
        SECTION("rgba") {
            REQUIRE(m.getValue("rgba", value));
            REQUIRE(value == glm::vec4(5.f, 6.f, 7.f, 8.f));
        }
        SECTION("stpq") {
            REQUIRE(m.getValue("stpq", value));
            REQUIRE(value == glm::vec4(5.f, 6.f, 7.f, 8.f));
        }
    }

    SECTION("glm::dvec4") {
        glm::dvec4 value = glm::dvec4(0.0);
        SECTION("n4") {
            REQUIRE(m.getValue("n4", value));
            REQUIRE(value == glm::dvec4(5.0, 6.0, 7.0, 8.0));
        }
        SECTION("num4") {
            REQUIRE(m.getValue("num4", value));
            REQUIRE(value == glm::dvec4(5.0, 6.0, 7.0, 8.0));
        }
        SECTION("xyzw") {
            REQUIRE(m.getValue("xyzw", value));
            REQUIRE(value == glm::dvec4(5.0, 6.0, 7.0, 8.0));
        }
        SECTION("rgba") {
            REQUIRE(m.getValue("rgba", value));
            REQUIRE(value == glm::dvec4(5.0, 6.0, 7.0, 8.0));
        }
        SECTION("stpq") {
            REQUIRE(m.getValue("stpq", value));
            REQUIRE(value == glm::dvec4(5.0, 6.0, 7.0, 8.0));
        }
    }

    SECTION("glm::ivec4") {
        glm::ivec4 value = glm::ivec4(0);
        SECTION("n4") {
            REQUIRE(m.getValue("n4", value));
            REQUIRE(value == glm::ivec4(5, 6, 7, 8));
        }
        SECTION("num4") {
            REQUIRE(m.getValue("num4", value));
            REQUIRE(value == glm::ivec4(5, 6, 7, 8));
        }
        SECTION("xyzw") {
            REQUIRE(m.getValue("xyzw", value));
            REQUIRE(value == glm::ivec4(5, 6, 7, 8));
        }
        SECTION("rgba") {
            REQUIRE(m.getValue("rgba", value));
            REQUIRE(value == glm::ivec4(5, 6, 7, 8));
        }
        SECTION("stpq") {
            REQUIRE(m.getValue("stpq", value));
            REQUIRE(value == glm::ivec4(5, 6, 7, 8));
        }
    }

    SECTION("glm::uvec4") {
        glm::uvec4 value = glm::uvec4(0);
        SECTION("n4") {
            REQUIRE(m.getValue("n4", value));
            REQUIRE(value == glm::uvec4(5, 6, 7, 8));
        }
        SECTION("num4") {
            REQUIRE(m.getValue("num4", value));
            REQUIRE(value == glm::uvec4(5, 6, 7, 8));
        }
        SECTION("xyzw") {
            REQUIRE(m.getValue("xyzw", value));
            REQUIRE(value == glm::uvec4(5, 6, 7, 8));
        }
        SECTION("rgba") {
            REQUIRE(m.getValue("rgba", value));
            REQUIRE(value == glm::uvec4(5, 6, 7, 8));
        }
        SECTION("stpq") {
            REQUIRE(m.getValue("stpq", value));
            REQUIRE(value == glm::uvec4(5, 6, 7, 8));
        }
    }

    SECTION("glm::bvec4") {
        glm::bvec4 value = glm::bvec4(false);
        SECTION("n4") {
            REQUIRE(m.getValue("n4", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
            REQUIRE(value.z);
            REQUIRE(value.w);
        }
        SECTION("num4") {
            REQUIRE(m.getValue("num4", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
            REQUIRE(value.z);
            REQUIRE(value.w);
        }
        SECTION("xyzw") {
            REQUIRE(m.getValue("xyzw", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
            REQUIRE(value.z);
            REQUIRE(value.w);
        }
        SECTION("rgba") {
            REQUIRE(m.getValue("rgba", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
            REQUIRE(value.z);
            REQUIRE(value.w);
        }
        SECTION("stpq") {
            REQUIRE(m.getValue("stpq", value));
            REQUIRE(value.x);
            REQUIRE(value.y);
            REQUIRE(value.z);
            REQUIRE(value.w);
        }
    }

    SECTION("mixed") {
        glm::vec3 value = glm::vec3(0.f);
        REQUIRE_FALSE(m.getValue("mix", value));
        REQUIRE(value == glm::vec3(0.f));
    }
}

TEST_CASE("LuaToDictionary: Vector Classes Set", "[luatodictionary]") {
    ghoul::Dictionary m;

    m.setValue("t.vec2", glm::vec2(5, 6));
    m.setValue("t.vec3", glm::vec3(5, 6, 7));
    m.setValue("t.vec4", glm::vec4(5, 6, 7, 8));
    m.setValue("t.dvec2", glm::dvec2(5, 6));
    m.setValue("t.dvec3", glm::dvec3(5, 6, 7));
    m.setValue("t.dvec4", glm::dvec4(5, 6, 7, 8));
    m.setValue("t.ivec2", glm::ivec2(5, 6));
    m.setValue("t.ivec3", glm::ivec3(5, 6, 7));
    m.setValue("t.ivec4", glm::ivec4(5, 6, 7, 8));
    m.setValue("t.uvec2", glm::uvec2(5, 6));
    m.setValue("t.uvec3", glm::uvec3(5, 6, 7));
    m.setValue("t.uvec4", glm::uvec4(5, 6, 7, 8));
    m.setValue("t.bvec2", glm::bvec2(true));
    m.setValue("t.bvec3", glm::bvec3(true));
    m.setValue("t.bvec4", glm::bvec4(true));


    SECTION("glm::vec2") {
        glm::vec2 value = glm::vec2(0.f);
        REQUIRE(m.getValue("t.vec2", value));
        REQUIRE(value == glm::vec2(5.f, 6.f));
    }

    SECTION("glm::dvec2") {
        glm::dvec2 value = glm::dvec2(0.0);
        REQUIRE(m.getValue("t.dvec2", value));
        REQUIRE(value == glm::dvec2(5.0, 6.0));
    }

    SECTION("glm::ivec2") {
        glm::ivec2 value = glm::ivec2(0);
        REQUIRE(m.getValue("t.ivec2", value));
        REQUIRE(value == glm::ivec2(5, 6));
    }

    SECTION("glm::uvec2") {
        glm::uvec2 value = glm::uvec2(0);
        REQUIRE(m.getValue("t.uvec2", value));
        REQUIRE(value == glm::uvec2(5, 6));
    }

    SECTION("glm::bvec2") {
        glm::bvec2 value = glm::bvec2(false);
        REQUIRE(m.getValue("t.bvec2", value));
        REQUIRE(value.x);
        REQUIRE(value.y);
    }

    SECTION("glm::vec3") {
        glm::vec3 value = glm::vec3(0.f);
        REQUIRE(m.getValue("t.vec3", value));
        REQUIRE(value == glm::vec3(5.f, 6.f, 7.f));
    }

    SECTION("glm::dvec3") {
        glm::dvec3 value = glm::dvec3(0.0);
        REQUIRE(m.getValue("t.dvec3", value));
        REQUIRE(value == glm::dvec3(5.0, 6.0, 7.0));
    }

    SECTION("glm::ivec3") {
        glm::ivec3 value = glm::ivec3(0);
        REQUIRE(m.getValue("t.ivec3", value));
        REQUIRE(value == glm::ivec3(5, 6, 7));
    }

    SECTION("glm::uvec3") {
        glm::uvec3 value = glm::uvec3(0);
        REQUIRE(m.getValue("t.uvec3", value));
        REQUIRE(value == glm::uvec3(5, 6, 7));
    }

    SECTION("glm::bvec3") {
        glm::bvec3 value = glm::bvec3(false);
        REQUIRE(m.getValue("t.bvec3", value));
        REQUIRE(value.x);
        REQUIRE(value.y);
        REQUIRE(value.z);
    }

    SECTION("glm::vec4") {
        glm::vec4 value = glm::vec4(0.f);
        REQUIRE(m.getValue("t.vec4", value));
        REQUIRE(value == glm::vec4(5.f, 6.f, 7.f, 8.f));
    }

    SECTION("glm::dvec4") {
        glm::dvec4 value = glm::dvec4(0.0);
        REQUIRE(m.getValue("t.dvec4", value));
        REQUIRE(value == glm::dvec4(5.0, 6.0, 7.0, 8.0));
    }

    SECTION("glm::ivec4") {
        glm::ivec4 value = glm::ivec4(0);
        REQUIRE(m.getValue("t.ivec4", value));
        REQUIRE(value == glm::ivec4(5, 6, 7, 8));
    }

    SECTION("glm::uvec4") {
        glm::uvec4 value = glm::uvec4(0);
        REQUIRE(m.getValue("t.uvec4", value));
        REQUIRE(value == glm::uvec4(5, 6, 7, 8));
    }

    SECTION("glm::bvec4") {
        glm::bvec4 value = glm::bvec4(false);
        REQUIRE(m.getValue("t.bvec4", value));
        REQUIRE(value.x);
        REQUIRE(value.y);
        REQUIRE(value.z);
        REQUIRE(value.w);
    }
}

TEST_CASE("LuaToDictionary: Matrix Classes Get", "[luatodictionary]") {
    ghoul::Dictionary m;
    REQUIRE_NOTHROW(ghoul::lua::loadDictionaryFromFile(absPath(_configuration5), m));

    SECTION("glm::mat2x2") {
        glm::mat2x2 value = glm::mat2x2(0.f);
        REQUIRE(m.getValue("m2x2", value));
        REQUIRE(value == glm::mat2x2(5.f, 6.f, 9.f, 10.f));
    }

    SECTION("glm::mat2x3") {
        glm::mat2x3 value = glm::mat2x3(0.f);
        REQUIRE(m.getValue("m2x3", value));

        REQUIRE(value == glm::mat2x3(5.f, 6.f, 9.f, 10.f, 13.f, 14.f));
    }

    SECTION("glm::mat2x4") {
        glm::mat2x4 value = glm::mat2x4(0.f);
        REQUIRE(m.getValue("m2x4", value));
        REQUIRE(value == glm::mat2x4(5.f, 6.f, 9.f, 10.f, 13.f, 14.f, 17.f, 18.f));
    }

    SECTION("glm::mat3x2") {
        glm::mat3x2 value = glm::mat3x2(0.f);
        REQUIRE(m.getValue("m3x2", value));
        REQUIRE(value == glm::mat3x2(5.f, 6.f, 7.f, 9.f, 10.f, 11.f));
    }

    SECTION("glm::mat3x3") {
        glm::mat3x3 value = glm::mat3x3(0.f);
        REQUIRE(m.getValue("m3x3", value));
        REQUIRE(value == glm::mat3x3(5.f, 6.f, 7.f, 9.f, 10.f, 11.f, 13.f, 14.f, 15.f));
    }

    SECTION("glm::mat3x4") {
        glm::mat3x4 value = glm::mat3x4(0.f);
        REQUIRE(m.getValue("m3x4", value));
        REQUIRE(value == glm::mat3x4(
            5.f, 6.f, 7.f, 9.f, 10.f, 11.f, 13.f, 14.f, 15.f, 17.f, 18.f, 19.f
        ));
    }

    SECTION("glm::mat4x2") {
        glm::mat4x2 value = glm::mat4x2(0.f);
        REQUIRE(m.getValue("m4x2", value));
        REQUIRE(value == glm::mat4x2(5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f));
    }

    SECTION("glm::mat4x3") {
        glm::mat4x3 value = glm::mat4x3(0.f);
        REQUIRE(m.getValue("m4x3", value));
        REQUIRE(value == glm::mat4x3(
            5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f
        ));
    }

    SECTION("glm::mat4x4") {
        glm::mat4x4 value = glm::mat4x4(0.f);
        REQUIRE(m.getValue("m4x4", value));
        REQUIRE(value == glm::mat4x4(
            5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f,
            14.f, 15.f, 16.f, 17.f, 18.f, 19.f, 20.f
        ));
    }

    SECTION("glm::dmat2x2") {
        glm::dmat2x2 value = glm::dmat2x2(0.0);
        REQUIRE(m.getValue("m2x2", value));
        REQUIRE(value == glm::dmat2x2(5.0, 6.0, 9.0, 10.0));
    }

    SECTION("glm::dmat2x3") {
        glm::dmat2x3 value = glm::dmat2x3(0.0);
        REQUIRE(m.getValue("m2x3", value));
        REQUIRE(value == glm::dmat2x3(5.0, 6.0, 9.0, 10.0, 13.0, 14.0));
    }

    SECTION("glm::dmat2x4") {
        glm::dmat2x4 value = glm::dmat2x4(0.0);
        REQUIRE(m.getValue("m2x4", value));
        REQUIRE(value == glm::dmat2x4(5.0, 6.0, 9.0, 10.0, 13.0, 14.0, 17.0, 18.0));
    }

    SECTION("glm::dmat3x2") {
        glm::dmat3x2 value = glm::dmat3x2(0.0);
        REQUIRE(m.getValue("m3x2", value));
        REQUIRE(value == glm::dmat3x2(5.0, 6.0, 7.0, 9.0, 10.0, 11.0));
    }

    SECTION("glm::dmat3x3") {
        glm::dmat3x3 value = glm::dmat3x3(0.0);
        REQUIRE(m.getValue("m3x3", value));
        REQUIRE(value == glm::dmat3x3(5.0, 6.0, 7.0, 9.0, 10.0, 11.0, 13.0, 14.0, 15.0));
    }

    SECTION("glm::dmat3x4") {
        glm::dmat3x4 value = glm::dmat3x4(0.0);
        REQUIRE(m.getValue("m3x4", value));
        REQUIRE(value == glm::dmat3x4(
            5.0, 6.0, 7.0, 9.0, 10.0, 11.0, 13.0, 14.0, 15.0, 17.0, 18.0, 19.0
        ));
    }

    SECTION("glm::dmat4x2") {
        glm::dmat4x2 value = glm::dmat4x2(0.0);
        REQUIRE(m.getValue("m4x2", value));
        REQUIRE(value == glm::dmat4x2(5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0));
    }

    SECTION("glm::dmat4x3") {
        glm::dmat4x3 value = glm::dmat4x3(0.0);
        REQUIRE(m.getValue("m4x3", value));
        REQUIRE(value == glm::dmat4x3(
            5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0
        ));
    }

    SECTION("glm::dmat4x4") {
        glm::dmat4x4 value = glm::dmat4x4(0.0);
        REQUIRE(m.getValue("m4x4", value));
        REQUIRE(value == glm::dmat4x4(
            5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0,
            14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0
        ));
    }
}

TEST_CASE("LuaToDictionary: MatrixClass SetValue", "[luatodictionary]") {
    ghoul::Dictionary m;

    m.setValue("f.m2x2", glm::mat2x2(5.f, 6.f, 9.f, 10.f));
    m.setValue("f.m2x3", glm::mat2x3(5.f, 6.f, 7.f, 9.f, 10.f, 11.f));
    m.setValue("f.m2x4", glm::mat2x4(5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f));
    m.setValue("f.m3x2", glm::mat3x2(5.f, 6.f, 9.f, 10.f, 13.f, 14.f));
    m.setValue("f.m3x3", glm::mat3x3(5.f, 6.f, 7.f, 9.f, 10.f, 11.f, 13.f, 14.f, 15.f));
    m.setValue(
        "f.m3x4",
        glm::mat3x4(5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f)
    );
    m.setValue("f.m4x2", glm::mat4x2(5.f, 6.f, 9.f, 10.f, 13.f, 14.f, 17.f, 18.f));
    m.setValue(
        "f.m4x3",
        glm::mat4x3(5.f, 6.f, 7.f, 9.f, 10.f, 11.f, 13.f, 14.f, 15.f, 17.f, 18.f, 19.f)
    );
    m.setValue(
        "f.m4x4",
        glm::mat4x4(
            5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f,
            13.f, 14.f, 15.f, 16.f, 17.f, 18.f, 19.f, 20.f
        )
    );

    SECTION("glm::mat2x2") {
        glm::mat2x2 value = glm::mat2x2(0.f);
        REQUIRE(m.getValue("m2x2", value));
        REQUIRE(value == glm::mat2x2(5.f, 6.f, 9.f, 10.f));
    }

    SECTION("glm::mat2x3") {
        glm::mat2x3 value = glm::mat2x3(0.f);
        REQUIRE(m.getValue("m2x3", value));
        REQUIRE(value == glm::mat2x3(5.f, 6.f, 9.f, 10.f, 13.f, 14.f));
    }

    SECTION("glm::mat2x4") {
        glm::mat2x4 value = glm::mat2x4(0.f);
        REQUIRE(m.getValue("m2x4", value));
        REQUIRE(value == glm::mat2x4(5.f, 6.f, 9.f, 10.f, 13.f, 14.f, 17.f, 18.f));
    }

    SECTION("glm::mat3x2") {
        glm::mat3x2 value = glm::mat3x2(0.f);
        REQUIRE(m.getValue("m3x2", value));
        REQUIRE(value == glm::mat3x2(5.f, 6.f, 7.f, 9.f, 10.f, 11.f));
    }

    SECTION("glm::mat3x3") {
        glm::mat3x3 value = glm::mat3x3(0.f);
        REQUIRE(m.getValue("m3x3", value));
        REQUIRE(value == glm::mat3x3(5.f, 6.f, 7.f, 9.f, 10.f, 11.f, 13.f, 14.f, 15.f));
    }

    SECTION("glm::mat3x4") {
        glm::mat3x4 value = glm::mat3x4(0.f);
        REQUIRE(m.getValue("m3x4", value));
        REQUIRE(value == glm::mat3x4(
            5.f, 6.f, 7.f, 9.f, 10.f, 11.f, 13.f, 14.f, 15.f, 17.f, 18.f, 19.f
        ));
    }

    SECTION("glm::mat4x2") {
        glm::mat4x2 value = glm::mat4x2(0.f);
        REQUIRE(m.getValue("m4x2", value));
        REQUIRE(value == glm::mat4x2(5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f));
    }

    SECTION("glm::mat4x3") {
        glm::mat4x3 value = glm::mat4x3(0.f);
        REQUIRE(m.getValue("m4x3", value));
        REQUIRE(value == glm::mat4x3(
            5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f
        ));
    }

    SECTION("glm::mat4x4") {
        glm::mat4x4 value = glm::mat4x4(0.f);
        REQUIRE(m.getValue("m4x4", value));
        REQUIRE(value == glm::mat4x4(
            5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f,
            14.f, 15.f, 16.f, 17.f, 18.f, 19.f, 20.f
        ));
    }

    m.setValue("d.m2x2", glm::dmat2x2(5.0, 6.0, 9.0, 10.0));
    m.setValue("d.m2x3", glm::dmat2x3(5.0, 6.0, 7.0, 9.0, 10.0, 11.0));
    m.setValue("d.m2x4", glm::dmat2x4(5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0));
    m.setValue("d.m3x2", glm::dmat3x2(5.0, 6.0, 9.0, 10.0, 13.0, 14.0));
    m.setValue("d.m3x3", glm::dmat3x3(5.0, 6.0, 7.0, 9.0, 10.0, 11.0, 13.0, 14.0, 15.0));
    m.setValue(
        "d.m3x4",
        glm::dmat3x4(5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0)
    );
    m.setValue(
        "d.m4x2", glm::dmat4x2(5.0, 6.0, 9.0, 10.0, 13.0, 14.0, 17.0, 18.0));
    m.setValue(
        "d.m4x3",
        glm::dmat4x3(5.0, 6.0, 7.0, 9.0, 10.0, 11.0, 13.0, 14.0, 15.0, 17.0, 18.0, 19.0)
    );
    m.setValue(
        "d.m4x4",
        glm::dmat4x4(
            5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0,
            13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0
        )
    );

    SECTION("glm::dmat2x2") {
        glm::dmat2x2 value = glm::dmat2x2(0.0);
        REQUIRE(m.getValue("d.m2x2", value));
        REQUIRE(value == glm::dmat2x2(5.0, 6.0, 9.0, 10.0));
    }

    SECTION("glm::dmat2x3") {
        glm::dmat2x3 value = glm::dmat2x3(0.0);
        REQUIRE(m.getValue("d.m2x3", value));
        REQUIRE(value == glm::dmat2x3(5.0, 6.0, 9.0, 10.0, 13.0, 14.0));
    }

    SECTION("glm::dmat2x4") {
        glm::dmat2x4 value = glm::dmat2x4(0.0);
        REQUIRE(m.getValue("d.m2x4", value));
        REQUIRE(value == glm::dmat2x4(5.0, 6.0, 9.0, 10.0, 13.0, 14.0, 17.0, 18.0));
    }

    SECTION("glm::dmat3x2") {
        glm::dmat3x2 value = glm::dmat3x2(0.0);
        REQUIRE(m.getValue("d.m3x2", value));
        REQUIRE(value == glm::dmat3x2(5.0, 6.0, 7.0, 9.0, 10.0, 11.0));
    }

    SECTION("glm::dmat3x3") {
        glm::dmat3x3 value = glm::dmat3x3(0.0);
        REQUIRE(m.getValue("d.m3x3", value));
        REQUIRE(value == glm::dmat3x3(5.0, 6.0, 7.0, 9.0, 10.0, 11.0, 13.0, 14.0, 15.0));
    }

    SECTION("glm::dmat3x4") {
        glm::dmat3x4 value = glm::dmat3x4(0.0);
        REQUIRE(m.getValue("d.m3x4", value));
        REQUIRE(value == glm::dmat3x4(
            5.0, 6.0, 7.0, 9.0, 10.0, 11.0, 13.0, 14.0, 15.0, 17.0, 18.0, 19.0
        ));
    }

    SECTION("glm::dmat4x2") {
        glm::dmat4x2 value = glm::dmat4x2(0.0);
        REQUIRE(m.getValue("d.m4x2", value));
        REQUIRE(value == glm::dmat4x2(5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0));
    }

    SECTION("glm::dmat4x3") {
        glm::dmat4x3 value = glm::dmat4x3(0.0);
        REQUIRE(m.getValue("d.m4x3", value));
        REQUIRE(value == glm::dmat4x3(
            5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0
        ));
    }

    SECTION("glm::dmat4x4") {
        glm::dmat4x4 value = glm::dmat4x4(0.0);
        REQUIRE(m.getValue("d.m4x4", value));
        REQUIRE(value == glm::dmat4x4(
            5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0,
            14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0
        ));
    }
}

#endif
