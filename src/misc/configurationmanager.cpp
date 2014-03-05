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

#include "misc/configurationmanager.h"

#include <type_traits>

#include <ghoul/lua/ghoul_lua.h>
#include <ghoul/filesystem/filesystem>
#include <ghoul/logging/logging>

#include <assert.h>
#include <fstream>
#include <iostream>
#include <iterator>

namespace {
    const std::string _loggerCat = "ConfigurationManager";
}

namespace ghoul {

namespace helper {
    void populateDictionary(lua_State* L, Dictionary* D);
} // namespace helper
    

ConfigurationManager::ConfigurationManager()
    : _state(nullptr), _dictionary(nullptr)
{
    LDEBUG("Creating ConfigurationManager");
    
    _dictionary = new Dictionary;
}

ConfigurationManager::~ConfigurationManager() {
#ifdef GHL_DEBUG
    if (_state != nullptr) {
        LWARNING("ConfigurationManager was not deinitialized");
        deinitialize();
    }
#endif
    LDEBUG("Destroying ConfigurationManager");
    delete _dictionary;
}

bool ConfigurationManager::initialize(const std::string& configurationScript) {
    assert(_state == nullptr);
    

    LDEBUG("Create Lua state");
    _state = luaL_newstate();
    if (_state == nullptr) {
        LFATAL("Error creating new Lua state: Memory allocation error");
        return false;
    }
    LDEBUG("Open libraries");
    luaL_openlibs(_state);
    
    if (configurationScript == "") {
        return true;
    }
    
    return loadConfiguration(configurationScript);
}

void ConfigurationManager::deinitialize() {
    assert(_state != nullptr);
    
    LDEBUG("Close Lua state");
    lua_close(_state);
    _state = nullptr;
}

bool ConfigurationManager::loadConfiguration(const std::string& filename, bool isConfiguration) {
    assert(_state != nullptr);

    if (filename == "") {
        LWARNING("Tried to load empty filepath. Aborting.");
        return false;
    }
    
    if( ! ghoul::filesystem::FileSystem::ref().fileExists(filename)) {
        LWARNING("Tried to load '" << filename << "'. File does not exist, aborting!");
        return false;
    }
    
    int status;
    if (isConfiguration) {
        LDEBUG("Loading configuration '" << filename << "'");
        
        // load the file into a string and prepend to give the following format
        // configuration={
        //     key1="string",
        //     key2=2
        // }
        std::ifstream t(filename);
        std::string lua_source((std::istreambuf_iterator<char>(t)),
                        std::istreambuf_iterator<char>());
        lua_source = "configuration="+lua_source;
        status = luaL_loadstring(_state, lua_source.c_str());
    } else {
        LDEBUG("Loading configuration script '" << filename << "'");
        
        // The file is a standalone script and can be loaded directly
        status = luaL_loadfile(_state, filename.c_str());
    }
    
    if (status != LUA_OK) {
        LFATAL("Error loading configuration script: " << lua_tostring(_state, -1));
        
        // Do we have to deinitialize?
        //deinitialize();
        return false;
    }
    
    LDEBUG("Executing configuration script");
    if (lua_pcall(_state, 0, LUA_MULTRET, 0)) {
        LFATAL("Error executing configuration script: " << lua_tostring(_state, -1));
        deinitialize();
        return false;
    }
    
    std::vector<std::string> dictionary_names = {"configuration", "config", "conf", "cfg", "c", "out", "dictionary", "dict"};
    for (auto var: dictionary_names) {
        lua_getglobal(_state, var.c_str());
        if (lua_istable(_state, -1)) {
            LDEBUG("Using '"<< var << "' as dictionary");
            break;
        }
    }
    
    // check if any of the config names succeeded
    if (!lua_istable(_state, -1)) {
        std::stringstream dictionary_list;
        std::copy(dictionary_names.begin(), dictionary_names.end(), std::ostream_iterator<std::string>(dictionary_list," "));
        LFATAL("Could not find configuration variable. Possible alternatives is: '" << dictionary_list.str() << "'");
        
        // Do we have to deinitialize?
        //deinitialize();
        return false;
    }
    
    // Populate the dictionary
    helper::populateDictionary(_state, _dictionary);
    
#ifndef NDEBUG
    // Print the keys
    //LDEBUG("_dictionary contains the following");
    //LDEBUG(std::endl << _dictionary->serializeToLuaString());
#endif

    return true;
}

std::vector<std::string> ConfigurationManager::keys(const std::string& location) {
    assert(_state != nullptr);

    return _dictionary->keys(location);
}

bool ConfigurationManager::hasKey(const std::string& key) {
    assert(_state != nullptr);

    return _dictionary->hasKey(key);
}
    
void ConfigurationManager::setValue(const std::string& key, const char* value) {
    const std::string v(value);
    setValue(key, v);
}

//
// Helpers
//
void helper::populateDictionary(lua_State* L, Dictionary* D) {
    lua_pushnil(L);
        
    while(lua_next(L, -2) != 0)
    {
        // get the key name
        std::string key = lua_tostring(L, -2);
        
        // check type and add to dictionary
        if(lua_isnumber(L, -1)) {
            double value =lua_tonumber(L, -1);
            double intpart;
            double floatpart = std::modf(value, &intpart);
            if (floatpart == 0.0) {
                int int_value = static_cast<int>(value);
                D->setValue(key, int_value);
            } else {
                D->setValue(key, value);
            }
        } else if (lua_isboolean(L, -1)) {
            bool value = lua_toboolean(L, -1);
            D->setValue(key, value);
        } else if (lua_isstring(L, -1)) {
            std::string value = lua_tostring(L, -1);
            D->setValue(key, value);
            
            // It is a table, recurivly add more dictionaries
        } else if (lua_istable(L, -1)) {
            Dictionary* d = new Dictionary;
            populateDictionary(L, d);
            D->setValue(key, d);
        }
        
        // get back up one level
        lua_pop(L, 1);
    }
}

} // namespace ghoul
