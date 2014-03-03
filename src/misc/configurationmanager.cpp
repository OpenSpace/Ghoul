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

namespace {
    const std::string _loggerCat = "ConfigurationManager";
}

namespace ghoul {

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
    
    if ( ! lua::lua_loadIntoDictionary(_state, _dictionary, filename, isConfiguration)) {
        deinitialize();
        initialize();
        return false;
    }
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


} // namespace ghoul
