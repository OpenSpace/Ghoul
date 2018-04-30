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

#include <ghoul/fmt.h>
#include <ghoul/lua/ghoul_lua.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/misc/invariants.h>
#include <type_traits>

namespace ghoul::lua {

namespace internal {
template <typename T>
void push(lua_State* L, T value) {
    // We have to handle the floating point types first in this as floats are able to be
    // converted to ints, which would remove their fractional part
    // Same goes for the const char* check before the std::is_pointer, since we want to
    // handle the specialized case first
    if constexpr (std::is_convertible_v<T, lua_Number>) {
        // The extra check is to remove compiler warnings about unnecessary conversions
        if constexpr (std::is_same_v<T, lua_Number>) {
            lua_pushnumber(L, std::move(value));
        }
        else {
            lua_pushnumber(L, static_cast<lua_Number&&>(std::move(value)));
        }
    }
    else if constexpr (std::is_convertible_v<T, lua_Integer>) {
        // The extra check is to remove compiler warnings about unnecessary conversions
        if constexpr (std::is_same_v<T, lua_Integer>) {
            lua_pushinteger(L, std::move(value));
        }
        else {
            lua_pushinteger(L, static_cast<lua_Integer&&>(std::move(value)));
        }
    }
    else if constexpr (std::is_same_v<T, bool>) {
        lua_pushboolean(L, value);
    }
    else if constexpr (std::is_same_v<T, nil_t>) {
        lua_pushnil(L);
    }
    else if constexpr (std::is_same_v<T, const char*>) {
        lua_pushstring(L, value);
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        lua_pushstring(L, value.c_str());
    }
    else if constexpr (std::is_pointer_v<T>) {
        lua_pushlightuserdata(L, reinterpret_cast<void*>(value));
    }
    else {
        static_assert(sizeof(T) == 0, "Unable to push type T onto the Lua stack");
    }
}

} // namespace internal

template <typename T>
T value(lua_State* L) {
    ghoul_precondition(L != nullptr, "L must not be nullptr");
    ghoul_precondition(lua_gettop(L) > 0, "Stack of L must not be empty");

    if constexpr (std::is_same_v<T, bool>) {
        if (lua_isboolean(L, -1)) {
            return lua_toboolean(L, -1) == 1;
        }
    }
    else if constexpr (std::is_same_v<T, lua_Number>) {
        if (lua_isnumber(L, -1)) {
            return lua_tonumber(L, -1);
        }
    }
    else if constexpr (std::is_same_v<T, lua_Integer>) {
        if (lua_isinteger(L, -1)) {
            return lua_tointeger(L, -1);
        }
    }
    else if constexpr (std::is_integral_v<T>) {
        if (lua_isinteger(L, -1)) {
            return static_cast<T>(lua_tointeger(L, -1));
        }
    }
    else if constexpr (std::is_floating_point_v<T>) {
        if (lua_isnumber(L, -1)) {
            return static_cast<T>(lua_tonumber(L, -1));
        }
    }
    else if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, std::string>) {
        if (lua_isstring(L, -1)) {
            return lua_tostring(L, -1);
        }
    }
    else if constexpr (std::is_same_v<T, ghoul::Dictionary>) {
        if (lua_istable(L, -1)) {
            ghoul::Dictionary d;
            ghoul::lua::luaDictionaryFromState(L, d);
            return d;
        }
    }
    else {
        static_assert(sizeof(T) == 0, "Unhandled type T");
    }

    // If we get this far, none of the previous return statements were hit
    throw LuaFormatException(fmt::format(
        "Requested type {} was not the expected type {}",
        typeid(T).name(),
        luaTypeToString(lua_type(L, -1))
    ));
}

template <typename T>
T value(lua_State* L, const char* name) {
    ghoul_precondition(L != nullptr, "L must not be nullptr");
    ghoul_precondition(name != nullptr, "name must not be nullptr");
    ghoul_precondition(strlen(name) > 0, "name must not be empty");

    lua_getglobal(L, name);
    return value<T>(L);
}

template <typename... Ts>
void push(lua_State* L, Ts... arguments) {
    ghoul_precondition(L != nullptr, "L must not be nullptr");

    (internal::push(L, arguments), ...);
}


} // namespace ghoul::lua
