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

#include <ghoul/filesystem/filesystem.h>

#include <sstream>
#include <fstream>
#include <iterator>

using namespace ghoul::logging;

namespace ghoul {
namespace lua {

FormattingException::FormattingException(const std::string& msg)
    : std::runtime_error(msg) {
}

namespace {

std::string luaTableToString(lua_State* state, bool& success) {
    static const int KEY = -2;
    static const int VAL = -1;

    success = true;
    if (!lua_istable(state, -1)) {
        success = false;
        return "";
    }

    std::stringstream result;
    lua_pushnil(state);

    result << "{ ";
    while ((lua_next(state, KEY) != 0) && success) {
        const int keyType = lua_type(state, KEY);
        switch (keyType) {
            case LUA_TNUMBER:
                result << lua_tonumber(state, KEY);
                break;
            case LUA_TSTRING:
                result << lua_tostring(state, KEY);
                break;
            default:
                LERRORC("luaTableToString", "Missing type: " << keyType);
                break;
        }

        result << " = ";
        if (lua_isstring(state, VAL))
            result << lua_tostring(state, VAL);
        else if (lua_isnumber(state, VAL))
            result << lua_tonumber(state, VAL);
        else if (lua_istable(state, VAL))
            result << luaTableToString(state, success);
        result << " ";
        lua_pop(state, 1);
    }
    result << "}";
    return result.str();
}

void populateDictionary(lua_State* state, Dictionary& dict, bool& isPureArrayWithNumbers) {
    static const int KEY = -2;
    static const int VAL = -1;

    enum class TableType {
        Undefined = 1,  // 001
        Map = 3,        // 010
        Array = 5       // 101
    };

    isPureArrayWithNumbers = true;

    TableType type = TableType::Undefined;

    lua_pushnil(state);
    while (lua_next(state, KEY) != 0) {
        // get the key name
        std::string key;
        const int keyType = lua_type(state, KEY);
        switch (keyType) {
            case LUA_TNUMBER:
                if (type == TableType::Map)
                     throw FormattingException(
                    "Dictionary can only contain a pure map or a pure array");
                type = TableType::Array;
                key = std::to_string(lua_tointeger(state, KEY));
                break;
            case LUA_TSTRING:
                if (type == TableType::Array)
                     throw FormattingException(
                          "Dictionary can only contain a pure map or a pure array");
                type = TableType::Map;
                isPureArrayWithNumbers = false;
                key = lua_tostring(state, KEY);
                break;
            default:
                LERRORC("luaTableToString", "Missing type: " << keyType);
                break;
        }

        // get the value
        switch (lua_type(state, VAL)) {
            case LUA_TNUMBER: {
                double value = lua_tonumber(state, VAL);
                double intpart;
                double floatpart = std::modf(value, &intpart);
                if (floatpart == 0.0) {
                    int int_value = static_cast<int>(value);
                    dict.setValue(key, int_value);
                } else 
                    dict.setValue(key, value);
                isPureArrayWithNumbers &= true;
            } break;
            case LUA_TBOOLEAN: {
                bool value = (lua_toboolean(state, VAL) == 1);
                dict.setValue(key, value);
                isPureArrayWithNumbers &= true;
            } break;
            case LUA_TSTRING: {
                std::string value = lua_tostring(state, VAL);
                dict.setValue(key, value);
                isPureArrayWithNumbers = false;
            } break;
            case LUA_TTABLE: {
                isPureArrayWithNumbers = false;
                Dictionary d;
                bool pureArray;
                populateDictionary(state, d, pureArray);
                dict.setValue(key, d);
            } break;
            default:
                throw FormattingException("Unknown type: " + std::to_string(lua_type(state, VAL)));
        }

        // get back up one level
        lua_pop(state, 1);
    }
}

void populateDictionary(lua_State* state, Dictionary& dict) {
    bool ignore;
    populateDictionary(state, dict, ignore);
}

}

std::string logStack(lua_State* state, LogManager::LogLevel level) {
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
                case LUA_TTABLE: {
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

bool loadDictionaryFromFile(const std::string& filename, ghoul::Dictionary& dictionary) {
    const static std::string _loggerCat = "lua_loadDictionary";
    static lua_State* state = nullptr;

    if (state == nullptr) {
        LDEBUG("Creating Lua state");
        state = luaL_newstate();
        if (state == nullptr) {
            LFATAL("Error creating new Lua state: Memory allocation error");
            return false;
        }
        LDEBUG("Open libraries");
        luaL_openlibs(state);
    }

    if (filename.empty()) {
        LERROR("Filename was empty");
        return false;
    }

    if (!FileSys.fileExists(absPath(filename))) {
        LERROR("File '" << absPath(filename) << "' did not exist");
        return false;
    }

    LDEBUG("Loading dictionary script '" << filename << "'");
    int status = luaL_loadfile(state, filename.c_str());
    if (status != LUA_OK) {
        LERROR("Error loading script: '" << lua_tostring(state, -1) << "'");
        return false;
    }

    LDEBUG("Executing script");
    if (lua_pcall(state, 0, LUA_MULTRET, 0)) {
        LERROR("Error executing script: " << lua_tostring(state, -1));
        return false;
    }

    if (lua_isnil(state, -1)) {
        LERROR("Error in script: '" << filename << "'. Script did not return anything.");
        return false;
    }

    if (!lua_istable(state, -1)) {
        LERROR("Error in script: '" << filename << "'. Script did not return a table.");
        return false;
    }

    populateDictionary(state, dictionary);

    // Clean up after ourselves by cleaning the stack
    lua_settop(state, 0);

    return true;
}

}  // namespace lua
}  // namespace ghoul
