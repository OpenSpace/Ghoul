/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
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

#include "lua/lua_helper.h"

#include "lua/ghoul_lua.h"

#include <sstream>

using namespace ghoul::logging;

namespace {
std::string luaTableToString(lua_State* state, bool& success) {
    success = true;
    if (!lua_istable(state, -1)) {
        success = false;
        return "";
    }

    std::stringstream result;
    lua_pushnil(state);

    result << "{ ";
    while ((lua_next(state, -2) != 0) && success) {
        result << lua_tostring(state, -2) << " = ";
        if (lua_isstring(state, -1))
            result << lua_tostring(state, -1);
        else if (lua_isnumber(state, -1))
            result << lua_tonumber(state, -1);
        else if (lua_istable(state, -1))
            result << luaTableToString(state, success);
        result << " ";
        lua_pop(state, 1);
    }
    result << "}";
    return result.str();
}
}

namespace ghoul {
namespace lua {

std::string lua_logStack(lua_State* state, LogManager::LogLevel level) {
    std::stringstream result;
    const int top = lua_gettop(state);
    if (top == 0)
        result << "Lua Stack (empty)";
    else {
        result << "Lua Stack\n";
        for (int i = 1; i <= top; ++i) {
            result << i << ": ";
            const int t = lua_type(state, i);
            switch (t) {
            case LUA_TSTRING:
                result << lua_tostring(state, i);
                break;
            case LUA_TBOOLEAN:
                result << (lua_toboolean(state, i) ? "true" : "false");
                break;
            case LUA_TNUMBER:
                result << lua_tonumber(state, i);
                break;
            case LUA_TTABLE:
                {
                    bool success;
                    result << luaTableToString(state, success);
                    break;
                }
            default:
                result << lua_typename(state, t);
                break;
            }
            result << "\n";
        }
    }
    const std::string& resultStr = result.str();
    LogMgr.logMessage(level, resultStr);
    return resultStr;
}

std::string lua_logTable(lua_State* state, LogManager::LogLevel level) {
    bool success;
    const std::string& message = luaTableToString(state, success);
    if (!success) {
        LERRORC("lua_logTable", "Lua state did not have a table at the top of the stack");
        return "";
    }
    LogMgr.logMessage(level, message);
    return message;
}

} // namespace lua
} // namespace ghoul