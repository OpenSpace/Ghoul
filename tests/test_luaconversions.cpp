/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2022                                                               *
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
#include <ghoul/lua/ghoul_lua.h>
#include <ghoul/lua/lua_helper.h>
#include <random>
#include <iostream>

namespace {
    constexpr int NumberFuzzTests = 1000;
} // namespace

TEST_CASE("LuaConversion: LuaExecution", "[luaconversion]") {
    lua_State* state = luaL_newstate();
    luaL_openlibs(state);

    const int status = luaL_loadstring(state, "");
    CHECK(status == LUA_OK);

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion", "[luaconversion]", bool, char, signed char,
    unsigned char, short, unsigned short, int, unsigned int, long, unsigned long,
    long long, unsigned long long, float, double, long double)
{
    using T = TestType;

    lua_State* state = luaL_newstate();
    luaL_openlibs(state);

    const T val(1);

    ghoul::lua::push(state, val);

    const T value = ghoul::lua::value<T>(state);
    CHECK(value == Approx(val));

    lua_close(state);
}

TEST_CASE("LuaConversion: String", "[luaconversion]") {
    lua_State* state = luaL_newstate();

    ghoul::lua::push(state, "value");

    const std::string value = ghoul::lua::value<std::string>(state);
    CHECK(value == "value");

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion - Glm Types", "[luaconversion]", glm::vec2, glm::vec3,
    glm::vec4, glm::dvec2, glm::dvec3, glm::dvec4, glm::ivec2, glm::ivec3, glm::ivec4,
    glm::uvec2, glm::uvec3, glm::uvec4, glm::mat2x2, glm::mat2x3, glm::mat2x4,
    glm::mat3x2, glm::mat3x3, glm::mat3x4, glm::mat4x2, glm::mat4x3, glm::mat4x4,
    glm::dmat2x2, glm::dmat2x3, glm::dmat2x4, glm::dmat3x2, glm::dmat3x3, glm::dmat3x4,
    glm::dmat4x2, glm::dmat4x3, glm::dmat4x4)
{
    using T = TestType;

    lua_State* state = luaL_newstate();
    luaL_openlibs(state);

    const T val(1);

    ghoul::lua::push(state, val);

    bool success = false;
    const T value = ghoul::lua::tryGetValue<T>(state, success);
    CHECK(success == true);
    CHECK(ghoul::to_string(value) == ghoul::to_string(val));
    CHECK(value == val);

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion Fuzz <short", "[luaconversion]", char, signed char,
    unsigned char)
{
    using T = TestType;

    lua_State* state = luaL_newstate();

    std::mt19937 gen(1337);
    std::uniform_int_distribution<short> dis(
        std::numeric_limits<T>::lowest(),
        std::numeric_limits<T>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen));

        ghoul::lua::push(state, val);

        const T value = ghoul::lua::value<T>(state);
        CHECK(value == val);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion Fuzz >short", "[luaconversion]", short, unsigned short,
    int, unsigned int)
{
    using T = TestType;

    lua_State* state = luaL_newstate();

    std::mt19937 gen(1337);
    std::uniform_int_distribution<T> dis(
        std::numeric_limits<T>::lowest(),
        std::numeric_limits<T>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen));

        ghoul::lua::push(state, val);

        const T value = ghoul::lua::value<T>(state);
        CHECK(value == val);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion Fuzz Limited Signed", "[luaconversion]", long,
    long long)
{
    using T = TestType;
    lua_State* state = luaL_newstate();

    std::mt19937 gen(1337);
    std::uniform_int_distribution<> dis(
        // We need to limit the range of values as Lua uses 'doubles' to store, and some
        // values will not be representable
        std::numeric_limits<int>::lowest(),
        std::numeric_limits<int>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen));

        ghoul::lua::push(state, val);

        const T value = ghoul::lua::value<T>(state);
        CHECK(value == val);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion Fuzz Limited Unsigned", "[luaconversion]",
    unsigned long, unsigned long long)
{
    using T = TestType;
    lua_State* state = luaL_newstate();

    std::mt19937 gen(1337);
    std::uniform_int_distribution<unsigned int> dis(
        // We need to limit the range of values as Lua uses 'doubles' to store, and some
        // values will not be representable
        std::numeric_limits<unsigned int>::lowest(),
        std::numeric_limits<unsigned int>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen));

        ghoul::lua::push(state, val);

        const T value = ghoul::lua::value<T>(state);
        CHECK(value == val);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion Float Fuzz", "[luaconversion]", float, double,
    long double)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_real_distribution<T> dis(
        0.f,
        std::numeric_limits<T>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen));

        ghoul::lua::push(state, val);

        const T value = ghoul::lua::value<T>(state);
        CHECK(value == Approx(val));

        if (typeid(T) == typeid(long double)) {
            if (Approx(value) != std::numeric_limits<long double>::infinity()) {
                CHECK(value == Approx(val));
            }
        }
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Vec2 Float Fuzz", "[luaconversion]", glm::vec2,
    glm::dvec2)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    //TOD: update range to avoid infinity
    std::mt19937 gen(1337);
    std::uniform_real_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Vec2 Fuzz", "[luaconversion]", glm::ivec2, glm::uvec2)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_int_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Vec3 Float Fuzz", "[luaconversion]", glm::vec3,
    glm::dvec3)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_real_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Vec3 Fuzz", "[luaconversion]", glm::ivec3, glm::uvec3)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_int_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Vec4 Float Fuzz", "[luaconversion]", glm::vec4,
    glm::dvec4)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_real_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen), dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Vec4 Fuzz", "[luaconversion]", glm::ivec4, glm::uvec4)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_int_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen), dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Mat2x2 Fuzz", "[luaconversion]", glm::mat2x2,
    glm::dmat2x2)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_real_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen), dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Mat3x3 Fuzz", "[luaconversion]", glm::mat3x3,
    glm::dmat3x3)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_real_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen),
            dis(gen), dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Mat4x4 Fuzz", "[luaconversion]", glm::mat4x4,
    glm::dmat4x4)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_real_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen),
            dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen),
            dis(gen), dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Mat2x3 & Mat3x2 Fuzz", "[luaconversion]", glm::mat2x3,
    glm::dmat2x3, glm::mat3x2, glm::dmat3x2)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_real_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Mat3x4 & Mat4x3 Fuzz", "[luaconversion]", glm::mat3x4,
    glm::dmat3x4, glm::mat4x3, glm::dmat4x3)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_real_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen),
            dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}

TEMPLATE_TEST_CASE("LuaConversion: Mat2x4 & Mat4x2 Fuzz", "[luaconversion]", glm::mat2x4,
    glm::dmat2x4, glm::mat4x2, glm::dmat4x2)
{
    lua_State* state = luaL_newstate();

    using T = TestType;

    std::mt19937 gen(1337);
    std::uniform_real_distribution<typename T::value_type> dis(
        std::numeric_limits<typename T::value_type>::min(),
        std::numeric_limits<typename T::value_type>::max()
    );

    for (int i = 0; i < NumberFuzzTests; ++i) {
        const T val = T(dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen),
            dis(gen), dis(gen));

        ghoul::lua::push(state, val);

        bool success = false;
        const T value = ghoul::lua::tryGetValue<T>(state, success);
        CHECK(success == true);
        CHECK(ghoul::to_string(value) == ghoul::to_string(val));
        CHECK(value == val);

        lua_pop(state, 1);
    }

    lua_close(state);
}
