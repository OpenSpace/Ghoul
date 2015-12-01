/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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

#include <ghoul/lua/lua_helper.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/lua/ghoul_lua.h>

#include <sstream>
#include <fstream>

using std::string;

namespace {
    
static const int KeyTableIndex = -2;
static const int ValueTableIndex = -1;

string luaTableToString(lua_State* state, bool& success, int tableLocation = KeyTableIndex);
    
string luaValueToString(lua_State* state, int location) {
    int type = lua_type(state, location);
    switch (type) {
        case LUA_TBOOLEAN:
            return std::to_string(lua_toboolean(state, location));
        case LUA_TNUMBER:
            return std::to_string(lua_tonumber(state, location));
        case LUA_TSTRING:
            return lua_tostring(state, location);
        case LUA_TTABLE:
        {
            bool success;
            return luaTableToString(state, success, location);
        }
        default:
            return ghoul::lua::luaTypeToString(type);
    }
}
    
string luaTableToString(lua_State* state, bool& success, int tableLocation) {
    success = true;
    if (!lua_istable(state, tableLocation)) {
        success = false;
        return "";
    }
    
    std::stringstream result;
    lua_pushvalue(state, tableLocation);
    lua_pushnil(state);
    
    result << "{ ";
    while (lua_next(state, -2) != 0) {
        result << luaValueToString(state, KeyTableIndex);
        result << " = ";
        result << luaValueToString(state, ValueTableIndex);
        result << ",  ";
        lua_pop(state, 1);
    }
    lua_pop(state, 1);
    result << "}";
    return result.str();
}
}

namespace ghoul {
namespace lua {

static lua_State* _state = nullptr;

FormattingException::FormattingException(string message)
    : RuntimeError(std::move(message), "Lua")
{}

string errorLocation(lua_State* L) {
    luaL_where(L, 1);
    return lua_tostring(L, -1);
}

string logStack(lua_State* state, ghoul::logging::LogManager::LogLevel level) {
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
                    result << luaTableToString(state, success, i);
                    break;
				case LUA_TTHREAD:
				case LUA_TUSERDATA:
				case LUA_TLIGHTUSERDATA:
				case LUA_TFUNCTION:
					result << luaTypeToString(t);
					break;
                }
                default:
                    result << lua_typename(state, t);
                    break;
            }
            result << "\n";
        }
    }
    const string& resultStr = result.str();
    LogMgr.logMessage(level, resultStr);
    return resultStr;
}

bool loadDictionaryFromFile(
    const string& filename,
    ghoul::Dictionary& dictionary,
    lua_State* state
    )
{
    const static string _loggerCat = "lua_loadDictionaryFromFile";

    if (state == nullptr) {
        if (_state == nullptr) {
            LDEBUG("Creating Lua state");
            _state = luaL_newstate();
            if (_state == nullptr) {
                LFATAL("Error creating new Lua state: Memory allocation error");
                return false;
            }
            LDEBUG("Open libraries");
            luaL_openlibs(_state);
        }
        state = _state;
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
    int status = luaL_loadfile(state, absPath(filename).c_str());
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

    luaDictionaryFromState(state, dictionary);

    // Clean up after ourselves by cleaning the stack
    lua_settop(state, 0);

    return true;
}

bool loadDictionaryFromString(const string& script, Dictionary& dictionary, lua_State* state) {
    const static string _loggerCat = "lua_loadDictionaryFromString";

    if (state == nullptr) {
        if (_state == nullptr) {
            LDEBUG("Creating Lua state");
            _state = luaL_newstate();
            if (_state == nullptr) {
                LFATAL("Error creating new Lua state: Memory allocation error");
                return false;
            }
            LDEBUG("Open libraries");
            luaL_openlibs(_state);
        }
        state = _state;
    }

    LDEBUG("Loading dictionary script '" << script.substr(0, 12) << "[...]'");
    int status = luaL_loadstring(state, script.c_str());
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
        LERROR("Error in script: '" << script.substr(0, 12)
                                    << "[...]'. Script did not return anything.");
        return false;
    }

    if (!lua_istable(state, -1)) {
        LERROR("Error in script: '" << script.substr(0, 12)
                                    << "[...]'. Script did not return a table.");
        return false;
    }

    luaDictionaryFromState(state, dictionary);

    // Clean up after ourselves by cleaning the stack
    lua_settop(state, 0);

    return true;
}
    
string luaTypeToString(int type) {
    switch (type) {
        case LUA_TNONE:
            return "None";
        case LUA_TNIL:
            return "Nil";
        case LUA_TBOOLEAN:
            return "Boolean";
        case LUA_TLIGHTUSERDATA:
            return "Light UserData";
        case LUA_TNUMBER:
            return "Number";
        case LUA_TSTRING:
            return "String";
        case LUA_TTABLE:
            return "Table";
        case LUA_TFUNCTION:
            return "Function";
        case LUA_TUSERDATA:
            return "UserData";
        case LUA_TTHREAD:
            return "Thread";
        default:
            return "";
    }
}

void luaDictionaryFromState(lua_State* state, Dictionary& dict) {
    enum class TableType {
        Undefined = 1,  // 001
        Map = 3,        // 010
        Array = 5       // 101
    };

    TableType type = TableType::Undefined;

    lua_pushnil(state);
    while (lua_next(state, KeyTableIndex) != 0) {
        // get the key name
        string key;
        const int keyType = lua_type(state, KeyTableIndex);
        switch (keyType) {
            case LUA_TNUMBER:
                if (type == TableType::Map)
                    throw FormattingException(
                          "Dictionary can only contain a pure map or a pure "
                          "array");

                type = TableType::Array;
                key = std::to_string(lua_tointeger(state, KeyTableIndex));
                break;
            case LUA_TSTRING:
                if (type == TableType::Array)
                    throw FormattingException(
                          "Dictionary can only contain a pure map or a pure "
                          "array");
                type = TableType::Map;
                key = lua_tostring(state, KeyTableIndex);
                break;
            default:
                LERRORC("luaTableToString", "Missing type: " << keyType);
                break;
        }

        // get the value
        switch (lua_type(state, ValueTableIndex)) {
            case LUA_TNUMBER: {
                double value = lua_tonumber(state, ValueTableIndex);
                dict.setValue(key, value);
            } break;
            case LUA_TBOOLEAN: {
                bool value = (lua_toboolean(state, ValueTableIndex) == 1);
                dict.setValue(key, value);
            } break;
            case LUA_TSTRING: {
                std::string value = lua_tostring(state, ValueTableIndex);
                dict.setValue(key, value);
            } break;
            case LUA_TTABLE: {
                Dictionary d;
                luaDictionaryFromState(state, d);
                dict.setValue(key, d);
            } break;
            default:
                throw FormattingException("Unknown type: "
                                      + std::to_string(lua_type(state, ValueTableIndex)));
        }

        // get back up one level
        lua_pop(state, 1);
    }
}

lua_State* createNewLuaState() {
    const string _loggerCat = "createNewLuaState";
    lua_State* s;
    LDEBUG("Creating Lua state");
    s = luaL_newstate();
    if (s == nullptr) {
        LFATAL("Error creating new Lua state: Memory allocation error");
        return nullptr;
    }
    LDEBUG("Open libraries");
    luaL_openlibs(s);
    return s;
}

void destroyLuaState(lua_State* state) {
    lua_close(state);
}

namespace internal {

void deinitializeGlobalState() {
    if (_state)
        lua_close(_state);
    _state = nullptr;
}

} // namespace internal

}  // namespace lua
}  // namespace ghoul
