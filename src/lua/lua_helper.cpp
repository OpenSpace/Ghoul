/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2016                                                               *
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
#include <ghoul/logging/logmanager.h>
#include <ghoul/lua/ghoul_lua.h>

#include <fmt/format.h>

#include <sstream>
#include <fstream>

using std::string;

namespace {
    
static const int KeyTableIndex = -2;
static const int ValueTableIndex = -1;

string luaTableToString(lua_State* state, int tableLocation = KeyTableIndex);
    
string luaValueToString(lua_State* state, int location) {
    ghoul_assert(state, "State must not be nullptr");
    
    int type = lua_type(state, location);
    switch (type) {
        case LUA_TBOOLEAN:
            return std::to_string(lua_toboolean(state, location));
        case LUA_TNUMBER:
            return std::to_string(lua_tonumber(state, location));
        case LUA_TSTRING:
            return lua_tostring(state, location);
        case LUA_TTABLE:
            return luaTableToString(state, location);
        default:
            return ghoul::lua::luaTypeToString(type);
    }
}
    
string luaTableToString(lua_State* state, int tableLocation) {
    ghoul_assert(state, "State must not be nullptr");
    ghoul_assert(lua_istable(state, tableLocation), "Lua object is not a table");
    
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
    
lua_State* staticLuaState() {
    if (_state == nullptr) {
        _state = createNewLuaState();
    }
    return _state;
}

LuaRuntimeException::LuaRuntimeException(string message)
    : RuntimeError(std::move(message), "Lua")
{}

LuaFormatException::LuaFormatException(string message, string file)
    : LuaRuntimeException(std::move(message))
    , filename(std::move(file))
{}
    
LuaLoadingException::LuaLoadingException(string error, string file)
    : LuaRuntimeException(fmt::format("Error loading script '{}': {}", file, error))
    , errorMessage(std::move(error))
    , filename(std::move(file))
{}
    
LuaExecutionException::LuaExecutionException(string error, string file)
    : LuaRuntimeException(fmt::format("Error executing script '{}': {}", file, error))
    , errorMessage(std::move(error))
    , filename(std::move(file))
{}

string errorLocation(lua_State* state) {
    ghoul_assert(state, "State must not be empty");
    
    luaL_where(state, 1);
    string result = lua_tostring(state, -1);
    lua_pop(state, 1);
    return result;
}

string stackInformation(lua_State* state) {
    ghoul_assert(state, "State must not be nullptr");
    
    std::stringstream result;
    int top = lua_gettop(state);
    if (top == 0) {
        result << "Lua Stack (empty)";
    }
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
                    result << luaTableToString(state, i);
                    break;
                case LUA_TTHREAD:
                case LUA_TUSERDATA:
                case LUA_TLIGHTUSERDATA:
                case LUA_TFUNCTION:
                    result << luaTypeToString(t);
                    break;
                default:
                    result << lua_typename(state, t);
                    break;
            }
            result << "\n";
        }
    }
    return result.str();
}

void loadDictionaryFromFile(const string& filename, ghoul::Dictionary& dictionary,
                                                                         lua_State* state)
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");
    ghoul_assert(FileSys.fileExists(absPath(filename)), "Filename must be a file");
    
    if (state == nullptr) {
        state = staticLuaState();
    }

    int status = luaL_loadfile(state, absPath(filename).c_str());
    if (status != LUA_OK) {
        throw LuaLoadingException(lua_tostring(state, -1), absPath(filename));
    }

    status = lua_pcall(state, 0, LUA_MULTRET, 0);
    if (status != LUA_OK) {
        throw LuaExecutionException(lua_tostring(state, -1), absPath(filename));
    }

    if (lua_isnil(state, -1)) {
        throw LuaFormatException("Script did not return anything", absPath(filename));
    }

    if (!lua_istable(state, -1)) {
        throw LuaFormatException("Script did not return a table", absPath(filename));
    }

    luaDictionaryFromState(state, dictionary);

    // Clean up after ourselves by cleaning the stack
    lua_settop(state, 0);
}
    
ghoul::Dictionary loadDictionaryFromFile(const string& filename, lua_State* state) {
    ghoul::Dictionary result;
    loadDictionaryFromFile(filename, result, state);
    return result;
}

void loadDictionaryFromString(const string& script, Dictionary& dictionary,
                                                                         lua_State* state)
{
    ghoul_assert(!script.empty(), "Script must not be empty");
    
    if (state == nullptr) {
        state = staticLuaState();
    }

    int status = luaL_loadstring(state, script.c_str());
    if (status != LUA_OK) {
        throw LuaLoadingException(lua_tostring(state, -1));
    }

    if (lua_pcall(state, 0, LUA_MULTRET, 0)) {
        throw LuaExecutionException(lua_tostring(state, -1));
    }

    if (lua_isnil(state, -1)) {
        throw LuaFormatException("Script did not return anything");
    }

    if (!lua_istable(state, -1)) {
        throw LuaFormatException("Script did not return a table");
    }

    luaDictionaryFromState(state, dictionary);

    // Clean up after ourselves by cleaning the stack
    lua_settop(state, 0);
}
    
Dictionary loadDictionaryFromString(const string& script, lua_State* state) {
    Dictionary result;
    loadDictionaryFromString(script, result, state);
    return result;
}
    
void luaDictionaryFromState(lua_State* state, Dictionary& dict) {
    enum class TableType {
        Undefined = 1,  // 001
        Map = 3,        // 010
        Array = 5       // 101
    };
    
    ghoul_assert(state, "State must not be nullptr");

    TableType type = TableType::Undefined;

    lua_pushnil(state);
    while (lua_next(state, KeyTableIndex) != 0) {
        // get the key
        string key;
        int keyType = lua_type(state, KeyTableIndex);
        switch (keyType) {
            case LUA_TNUMBER:
                if (type == TableType::Map) {
                    throw LuaFormatException(
                          "Dictionary can only contain a pure map or a pure array"
                    );
                }

                type = TableType::Array;
                key = std::to_string(lua_tointeger(state, KeyTableIndex));
                break;
            case LUA_TSTRING:
                if (type == TableType::Array) {
                    throw LuaFormatException(
                          "Dictionary can only contain a pure map or a pure array"
                    );
                }
                
                type = TableType::Map;
                key = lua_tostring(state, KeyTableIndex);
                break;
            default:
                throw LuaFormatException("Table index type is not a number or a string");
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
                throw LuaFormatException("Unknown type: "
                                      + std::to_string(lua_type(state, ValueTableIndex)));
        }

        // get back up one level
        lua_pop(state, 1);
    }
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
    }
    ghoul_assert(false, "Missing type in switch-case");
}

lua_State* createNewLuaState() {
    LDEBUGC("Lua", "Creating Lua state");
    lua_State* s = luaL_newstate();
    if (s == nullptr) {
        throw LuaRuntimeException("Error creating Lua state: Memory allocation");
    }
    LDEBUGC("Lua", "Open libraries");
    luaL_openlibs(s);
    return s;
}

void destroyLuaState(lua_State* state) {
    ghoul_assert(state, "State must not be nullptr");
    lua_close(state);
}
    
void runScriptFile(lua_State* state, const std::string& filename) {
    ghoul_assert(state, "State must not be nullptr");
    ghoul_assert(!filename.empty(), "Filename must not be empty");
    ghoul_assert(FileSys.fileExists(absPath(filename)), "Filename must be a file");
    
    int status = luaL_loadfile(state, filename.c_str());
    if (status != LUA_OK) {
        throw LuaLoadingException(lua_tostring(state, -1));
    }
    
    if (lua_pcall(state, 0, LUA_MULTRET, 0)) {
        throw LuaExecutionException(lua_tostring(state, -1));
    }
}

void runScript(lua_State* state, const std::string& script) {
    ghoul_assert(state, "State must not be nullptr");
    ghoul_assert(!script.empty(), "Script must not be empty");
    
    int status = luaL_loadstring(state, script.c_str());
    if (status != LUA_OK) {
        throw LuaLoadingException(lua_tostring(state, -1));
    }
    
    if (lua_pcall(state, 0, LUA_MULTRET, 0)) {
        throw LuaExecutionException(lua_tostring(state, -1));
    }
}

namespace internal {

void deinitializeGlobalState() {
    if (_state) {
        lua_close(_state);
    }
    _state = nullptr;
}

} // namespace internal

}  // namespace lua
}  // namespace ghoul
