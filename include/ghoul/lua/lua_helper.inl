/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2021                                                               *
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
#include <ghoul/glm.h>
#include <ghoul/lua/ghoul_lua.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/defer.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/misc/invariants.h>
#include <filesystem>
#include <optional>
#include <type_traits>

namespace ghoul::lua {

namespace internal {

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <size_t I = 0, typename... Ts>
constexpr void extractValues(lua_State* L, std::tuple<Ts...>& tuple, int baseLocation,
                             int nArguments)
{
    using T = std::tuple_element_t<I, std::tuple<Ts...>>;

    if constexpr (is_optional<T>::value) {
        if (baseLocation + I > nArguments) {
            // We have reached the end of the arguments and only have optional now, so we
            // can bail out
            return;
        }
        else {
            // We are looking at an optional parameter that is provided
            std::get<I>(tuple) = value<T::value_type>(L, baseLocation + I, PopValue::No);
        }
    }
    else {
        std::get<I>(tuple) = value<T>(L, baseLocation + I, PopValue::No);
    }

    if constexpr (I+1 != sizeof...(Ts)) {
        extractValues<I+1>(L, tuple, baseLocation, nArguments);
    }
}

enum class Phase {
    Mandatory,
    Optional
};

template <size_t I = 0, typename... Ts>
constexpr bool verifyParameters(Phase phase = Phase::Mandatory) {
    using T = std::tuple_element_t<I, std::tuple<Ts...>>;

    if constexpr (is_optional<T>::value) {
        phase = Phase::Optional;
    }
    else {
        if (phase == Phase::Optional) {
            // We are currently in the optional phase, but we have a non-optional type,
            // which is not allowed
            return false;
        }
    }

    if constexpr (I+1 != sizeof...(Ts)) {
        return verifyParameters<I+1, Ts...>(phase);
    }
    else {
        // We have reached the end without a single failure -> success
        return true;
    }
}

template <typename T>
void push(lua_State* L, T value) {
    // We have to handle the floating point types first in this as floats are able to be
    // converted to ints, which would remove their fractional part
    // Same goes for the const char* check before the std::is_pointer, since we want to
    // handle the specialized case first
    if constexpr (std::is_same_v<T, bool>) {
        lua_pushboolean(L, value ? 1 : 0);
    }
    else if constexpr (std::is_same_v<T, lua_Number>) {
        lua_pushnumber(L, std::move(value));
    }
    else if constexpr (std::is_same_v<T, lua_Integer>) {
        lua_pushinteger(L, std::move(value));
    }
    else if constexpr (std::is_floating_point_v<T>) {
        lua_pushnumber(L, static_cast<lua_Number&&>(std::move(value)));
    }
    else if constexpr (std::is_integral_v<T>) {
        lua_pushinteger(L, static_cast<lua_Integer&&>(std::move(value)));
    }
    else if constexpr (std::is_same_v<T, nil_t>) {
        lua_pushnil(L);
    }
    else if constexpr (std::is_same_v<T, const char*>) {
        lua_pushstring(L, value);
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        lua_pushlstring(L, value.c_str(), value.size());
    }
    else if constexpr (std::is_same_v<T, std::string_view>) {
        lua_pushlstring(L, value.data(), value.size());
    }
    else if constexpr (std::is_same_v<T, std::filesystem::path>) {
        lua_pushlstring(L, value.string().c_str(), value.string().size());
    }
    else if constexpr (std::is_same_v<T, std::vector<double>>
        || std::is_same_v<T, std::vector<float>>)
    {
        lua_newtable(L);
        for (size_t i = 0; i < value.size(); ++i) {
            lua_pushinteger(L, i + 1);
            lua_pushnumber(L, std::move(value[i]));
            lua_settable(L, -3);
        }
    }
    else if constexpr (std::is_same_v<T, std::vector<int>>) {
        lua_newtable(L);
        for (size_t i = 0; i < value.size(); ++i) {
            lua_pushinteger(L, i + 1);
            lua_pushinteger(L, std::move(value[i]));
            lua_settable(L, -3);
        }
    }
    else if constexpr (std::is_same_v<T, std::vector<const char*>>) {
        lua_newtable(L);
        for (size_t i = 0; i < value.size(); ++i) {
            lua_pushinteger(L, i + 1);
            lua_pushstring(L, value[i]);
            lua_settable(L, -3);
        }
    }
    else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
        lua_newtable(L);
        for (size_t i = 0; i < value.size(); ++i) {
            lua_pushinteger(L, i + 1);
            lua_pushstring(L, value[i].c_str());
            lua_settable(L, -3);
        }
    }
    else if constexpr (std::is_pointer_v<T>) {
        lua_pushlightuserdata(L, reinterpret_cast<void*>(value));
    }
    else if constexpr (isGlmVector<T>()) {
        lua_newtable(L);
        int number = 1;
        for (glm::length_t i = 0; i < ghoul::glm_components<T>::value; ++i) {
            lua_pushnumber(L, static_cast<lua_Number>(value[i]));
            lua_rawseti(L, -2, number);
            ++number;
        }
    }
    else if constexpr (isGlmMatrix<T>()) {
        lua_newtable(L);
        int number = 1;
        for (glm::length_t i = 0; i < T::type::row_type::length(); ++i) {
            for (glm::length_t j = 0; j < T::type::col_type::length(); ++j) {
                lua_pushnumber(L, static_cast<lua_Number>(value[i][j]));
                lua_rawseti(L, -2, number);
                ++number;
            }
        }
    }
    else {
        static_assert(sizeof(T) == 0, "Unable to push type T onto the Lua stack");
    }
}

template <typename T>
T value(lua_State* L, int location) {
    if constexpr (std::is_same_v<T, bool>) {
        if (lua_isboolean(L, location)) {
            return lua_toboolean(L, location) == 1;
        }
    }
    else if constexpr (std::is_same_v<T, lua_Number>) {
        if (lua_isnumber(L, location)) {
            return lua_tonumber(L, location);
        }
    }
    else if constexpr (std::is_same_v<T, lua_Integer>) {
        if (lua_isinteger(L, location)) {
            return lua_tointeger(L, location);
        }
    }
    else if constexpr (std::is_integral_v<T>) {
        if (lua_isinteger(L, location)) {
            return static_cast<T>(lua_tointeger(L, location));
        }
    }
    else if constexpr (std::is_floating_point_v<T>) {
        if (lua_isnumber(L, location)) {
            return static_cast<T>(lua_tonumber(L, location));
        }
    }
    else if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, std::string>) {
        if (lua_isstring(L, location)) {
            return lua_tostring(L, location);
        }
    }
    else if constexpr (std::is_same_v<T, ghoul::Dictionary>) {
        if (lua_istable(L, location)) {
            lua_pushvalue(L, location);
            defer { lua_pop(L, 1); };

            ghoul::Dictionary d;
            ghoul::lua::luaDictionaryFromState(L, d);

            return d;
        }
    }
    else {
        static_assert(sizeof(T) == 0, "Unhandled type T");
    }

    std::string error = fmt::format(
        "Requested type {} for parameter {} was not the expected type {}",
        typeid(T).name(),
        location,
        luaTypeToString(lua_type(L, location))
    );

    ghoul_assert(false, error);
    // If we get this far, none of the previous return statements were hit
    throw LuaFormatException(std::move(error));
}

} // namespace internal

template <typename T>
T value(lua_State* L, int location, PopValue shouldPopValue) {
    ghoul_precondition(L != nullptr, "L must not be nullptr");

    T res = internal::value<T>(L, location);
    if (shouldPopValue) {
        lua_pop(L, 1);
    }
    return res;
}

template <typename T>
T value(lua_State* L, const char* name, PopValue shouldPopValue) {
    ghoul_precondition(L != nullptr, "L must not be nullptr");
    ghoul_precondition(name != nullptr, "name must not be nullptr");
    ghoul_precondition(strlen(name) > 0, "name must not be empty");

    lua_getglobal(L, name);

    T res = value<T>(L);

    if (shouldPopValue) {
        lua_pop(L, 1);
    }

    return res;
}

template <typename... Ts>
constexpr void values(lua_State* L, std::tuple<Ts...>& tuple, int location) {
    ghoul_precondition(L != nullptr, "L must not be nullptr");

    // Verify that we don't have a situation where we have non-optional parameters, then
    // optional parameters, and then non-optional parameters again as that will bork up
    // the argument calculations
    static_assert(
        internal::verifyParameters<0, Ts...>(),
        "Tuple parameters have to list all non-optional parameters first, then the "
        "optional ones and cannot get back to non-optional parameters"
    );

    int n = lua_gettop(L);
    internal::extractValues(L, tuple, location, n);
}

template <typename... Ts>
void push(lua_State* L, Ts... arguments) {
    ghoul_precondition(L != nullptr, "L must not be nullptr");

    (internal::push(L, arguments), ...);
}

template <typename T>
T tryGetValue(lua_State* L, bool& success) {
    ghoul_precondition(L != nullptr, "L must not be nullptr");

    const T fallback = T();
    T result = fallback;

    if constexpr (isGlmVector<T>()) {
        lua_pushnil(L);
        for (glm::length_t i = 0; i < ghoul::glm_components<T>::value; ++i) {
            int hasNext = lua_next(L, -2);
            if (hasNext != 1) {
                success = false;
                return fallback;
            }
            if (lua_isnumber(L, -1) != 1) {
                success = false;
                return fallback;
            }
            else {
                result[i] = static_cast<typename T::value_type>(lua_tonumber(L, -1));
                lua_pop(L, 1);
            }
        }
        // The last accessor argument and the table are still on the stack
        lua_pop(L, 1);
        success = true;
    }
    else if constexpr (isGlmMatrix<T>()) {
        lua_pushnil(L);
        int number = 1;
        for (glm::length_t i = 0; i < T::type::row_type::length(); ++i) {
            for (glm::length_t j = 0; j < T::type::col_type::length(); ++j) {
                int hasNext = lua_next(L, -2);
                if (hasNext != 1) {
                    success = false;
                    return fallback;
                }
                if (lua_isnumber(L, -1) != 1) {
                    success = false;
                    return fallback;
                }
                else {
                    result[i][j] =
                        static_cast<typename T::value_type>(lua_tonumber(L, -1));
                    lua_pop(L, 1);
                    ++number;
                }
            }
        }
        // The last accessor argument and the table are still on the stack
        lua_pop(L, 1);
        success = true;
    }
    else {
        static_assert(sizeof(T) == 0, "Unable to push type T onto the Lua stack");
    }

    return result;
}

} // namespace ghoul::lua
