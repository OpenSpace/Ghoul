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
    

#define KEY -2
#define VAL -1
void luaPrivate_populateDictionary(lua_State* L, Dictionary* D) {
    lua_pushnil(L);
    
    while(lua_next(L, KEY) != 0)
    {
        
        // get the key name
        std::string key;
        switch (lua_type(L, KEY)) {
            case LUA_TNUMBER:
                {
                    std::stringstream ss;
                    unsigned int ikey = lua_tounsigned(L,KEY);
                    //lua_pop(L, 1);
                    ss << ikey;
                    key = ss.str();
                }
                break;
            case LUA_TSTRING:
                key = lua_tostring(L,KEY);
                break;
        }
        
        // get the value
        switch (lua_type(L, VAL)) {
            case LUA_TNIL:
            {
                D->setValue(key, nullptr);
            }
                break;
            case LUA_TNUMBER:
            {
                double value = lua_tonumber(L, VAL);
                D->setValue(key, value);
            }
                break;
            case LUA_TBOOLEAN:
            {
                bool value = (lua_toboolean(L, VAL) == 1);
                D->setValue(key, value);
            }
                break;
            case LUA_TSTRING:
            {
                std::string value = lua_tostring(L, VAL);
                D->setValue(key, value);
            }
                break;
            case LUA_TTABLE:
            {
                Dictionary d;
                luaPrivate_populateDictionary(L, &d);
                D->setValue(key, std::move(d));
            }
                break;
        }
        
        // get back up one level
        lua_pop(L, 1);
    }
}

bool lua_loadIntoDictionary(lua_State* state, ghoul::Dictionary* D, const std::string& filename, bool isConfiguration) {
    assert(state != nullptr);
    
    if (filename == "") {
        LWARNINGC("lua_loadIntoDictionary","Tried to load empty filepath. Aborting.");
        return false;
    }
    
    if( ! ghoul::filesystem::FileSystem::ref().fileExists(filename)) {
        LWARNINGC("lua_loadIntoDictionary","Tried to load '" << filename << "'. File does not exist, aborting!");
        return false;
    }
    
    int status;
    if (isConfiguration) {
        LDEBUGC("lua_loadIntoDictionary","Loading configuration '" << filename << "'");
        
        // load the file into a string and prepend to give the following format
        // configuration={
        //     key1="string",
        //     key2=2
        // }
        std::ifstream t(filename);
        std::string lua_source((std::istreambuf_iterator<char>(t)),
                               std::istreambuf_iterator<char>());
        lua_source = "configuration="+lua_source;
        status = luaL_loadstring(state, lua_source.c_str());
    } else {
        LDEBUGC("lua_loadIntoDictionary","Loading configuration script '" << filename << "'");
        
        // The file is a standalone script and can be loaded directly
        status = luaL_loadfile(state, filename.c_str());
    }
    
    if (status != LUA_OK) {
        LFATALC("lua_loadIntoDictionary","Error loading configuration script: " << lua_tostring(state, -1));
        return false;
    }
    
    LDEBUGC("lua_loadIntoDictionary","Executing configuration script");
    if (lua_pcall(state, 0, LUA_MULTRET, 0)) {
        LFATALC("lua_loadIntoDictionary","Error executing configuration script: " << lua_tostring(state, -1));
        return false;
    }
    
    std::vector<std::string> dictionary_names = {"configuration", "config", "conf", "cfg", "c", "out", "dictionary", "dict"};
    for (auto var: dictionary_names) {
        lua_getglobal(state, var.c_str());
        if (lua_istable(state, -1)) {
            LDEBUGC("lua_loadIntoDictionary","Using '"<< var << "' as dictionary");
            break;
        }
    }
    
    // check if any of the config names succeeded
    if (!lua_istable(state, -1)) {
        std::stringstream dictionary_list;
        std::copy(dictionary_names.begin(), dictionary_names.end(), std::ostream_iterator<std::string>(dictionary_list," "));
        LFATALC("lua_loadIntoDictionary","Could not find configuration variable. Possible alternatives is: '" << dictionary_list.str() << "'");
        
        // Do we have to deinitialize?
        //deinitialize();
        return false;
    }
    
    // Populate the dictionary
    luaPrivate_populateDictionary(state, D);
    
#ifndef NDEBUG
    // Print the keys
    //LDEBUGC("lua_loadIntoDictionary","_dictionary contains the following");
    //LDEBUGC("lua_loadIntoDictionary",std::endl << D->serializeToLuaString());
#endif
    
    return true;
}

} // namespace lua
} // namespace ghoul
