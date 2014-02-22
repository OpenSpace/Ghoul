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

#include <ghoul/lua/ghoul_lua.h>
#include <ghoul/filesystem/filesystem>
#include <ghoul/logging/logging>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assert.h>
#include <fstream>

namespace {
    const std::string _loggerCat = "ConfigurationManager";
    const char* _configurationTable = "config";

    const std::string _configurationScript = "\
config = {} \n \
function merge(t1, t2) \n \
    for k, v in pairs(t2) do \n \
        if (type(v) == 'table') and (type(t1[k] or false) == 'table') then \n \
            merge(t1[k], t2[k]) \n \
        else \n \
            t1[k] = v \n \
        end \n \
    end \n \
    return t1 \n \
end \n \
\n \
function createTables(key, t) \n \
    pos = key:find('[.]') \n \
    if (not pos) then \n \
        pos = key:find('[[]') \n \
        if (pos) then \n \
            key = key:sub(0,pos-1) \n \
        end \n \
        if (not t[key]) then \n \
            t[key] = {} \n \
        end \n \
    else \n \
        newKey = key:sub(0, pos - 1) \n \
        newTable = t[newKey] \n \
        if (not newTable) then \n \
            newTable = {} \n \
            t[newKey] = newTable \n \
        end \n \
        createTables(key:sub(pos + 1), newTable) \n \
    end \n \
end \n \
\n \
function loadConfiguration(file) \n \
    io.input(file) \n \
    contents = io.read('*all') \n \
    source = 'return ' .. contents \n \
    settings = assert(load(source))() \n \
    merge(config, settings or {}) \n \
end \n \
\n \
function getKeys(location, t) \n \
    t = t or config \n \
    if (location == '') then \n \
        result = {} \n \
        n = 1 \n \
        for k,_ in pairs(t) do \n \
            result[n] = k \n \
            n = n + 1 \n \
        end \n \
        return result \n \
    else \n \
        pos = location:find('[.]') \n \
        if (not pos) then \n \
            newTable = t[location] \n \
            if (newTable) then \n \
                result = {} \n \
                n = 1 \n \
                for k,_ in pairs(newTable) do \n \
                    result[n] = k \n \
                    n = n + 1 \n \
                end \n \
                return result \n \
            else \n \
                -- 'location' does not point to a valid table \n \
                return nil \n \
            end \n \
        else \n \
            newTable = t[location:sub(0, pos - 1)] \n \
            if (not newTable) then \n \
                -- 'location' does not point to a valid table \n \
                return nil \n \
            else \n \
                return getKeys(location:sub(pos + 1), newTable) \n \
            end \n \
        end \n \
    end \n \
end \n \
\n \
function hasKey(key, t) \n \
    return getValue(key, config) \n \
end \n \
\n \
function getValue(key, t) \n \
    return assert(load('return config.' .. key))() \n \
end \n \
\n \
function setValue(key, v, t) \n \
    createTables(key, config) \n \
    assert(load('local x=... config.' .. key .. ' = x'))(v) \n \
end \n \
";
}

namespace ghoul {

namespace helper {
    bool getValue(lua_State* state, const std::string& key, int& value);
    bool getValue(lua_State* state, const std::string& key, lua_Integer& value);
    bool getValue(lua_State* state, const std::string& key, lua_Unsigned& value);
    bool getValue(lua_State* state, const std::string& key, lua_Number& value);
    void setValue(lua_State* state, const std::string& key, const lua_Integer& value);
    void setValue(lua_State* state, const std::string& key, const lua_Unsigned& value);
    void setValue(lua_State* state, const std::string& key, const lua_Number& value);
}

ConfigurationManager::ConfigurationManager()
    : _state(nullptr)
{
    LDEBUG("Creating ConfigurationManager");
}

ConfigurationManager::~ConfigurationManager() {
#ifdef GHL_DEBUG
    if (_state != nullptr) {
        LWARNING("ConfigurationManager was not deinitialized");
        deinitialize();
    }
#endif
    LDEBUG("Destroying ConfigurationManager");
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

    if (configurationScript != "") {
        //const std::string absConfigurationScript = absPath(configurationScript);
        LDEBUG("Loading configuration script '" << configurationScript << "'");
        const int status = luaL_loadfile(_state, configurationScript.c_str());
        if (status != LUA_OK) {
            LFATAL("Error loading configuration script" << lua_tostring(_state, -1));
            deinitialize();
            return false;
        }
    }
    else {
        LDEBUG("Loading hard-coded configuration script");
        const int status = luaL_loadstring(_state, _configurationScript.c_str());
        if (status != LUA_OK) {
            LFATAL("Error loading configuration script" << lua_tostring(_state, -1));
            deinitialize();
            return false;
        }
    }

    LDEBUG("Executing configuration script");
    if (lua_pcall(_state, 0, LUA_MULTRET, 0)) {
        LFATAL("Error executing configuration script: " << lua_tostring(_state, -1));
        deinitialize();
        return false;
    }

    // Sanity checks
    LDEBUG("Sanity check for 'loadConfiguration'");
    lua_getglobal(_state, "loadConfiguration");
    if (!lua_isfunction(_state, -1)) {
        LFATAL("Could not find 'loadConfiguration' in configuration script");
        deinitialize();
        return false;
    }
    lua_pop(_state, 1);
    
    LDEBUG("Sanity check for the configuration table");
    lua_getglobal(_state, _configurationTable);
    if (!lua_istable(_state, -1)) {
        LERROR("'" << _configurationTable << "' not found in configuration script");
        deinitialize();
        return false;
    }
    lua_pop(_state, 1);

    LDEBUG("Sanity check for 'getValue'");
    lua_getglobal(_state, "getValue");
    if (!lua_isfunction(_state, -1)) {
        LFATAL("Could not find 'getValue' in configuration script");
        deinitialize();
        return false;
    }
    lua_pop(_state, 1);

    LDEBUG("Sanity check for 'setValue'");
    lua_getglobal(_state, "setValue");
    if (!lua_isfunction(_state, -1)) {
        LFATAL("Could not find 'setValue' in configuration script");
        deinitialize();
        return false;
    }
    lua_pop(_state, 1);

    LDEBUG("Sanity check for 'getKeys'");
    lua_getglobal(_state, "getKeys");
    if (!lua_isfunction(_state, -1)) {
        LFATAL("Could not find 'getKeys' in configuration script");
        deinitialize();
        return false;
    }
    lua_pop(_state, 1);

    return true;
}

void ConfigurationManager::deinitialize() {
    assert(_state != nullptr);
    LDEBUG("Close Lua state");
    lua_close(_state);
    _state = nullptr;
}

bool ConfigurationManager::loadConfiguration(const std::string& filename) {
    assert(_state != nullptr);

    const std::string& absFilename = absPath(filename);
    LDEBUG("Loading configuration: " << absFilename);

    lua_getglobal(_state, "loadConfiguration");
    lua_pushstring(_state, absFilename.c_str());

    LDEBUG("Calling loadConfiguration function");
    const int status = lua_pcall(_state, 1, 0, 0);
    if (status != LUA_OK) {
        LERROR("Error loading configuration: " << lua_tostring(_state, -1));
        return false;
    }

    return true;
}

// TODO: We can replace this by using type_traits in a smart way

template<>
void ConfigurationManager::setValue(const std::string& key, const bool& value) {
    assert(_state != nullptr);

    lua_getglobal(_state, "setValue");
    lua_pushstring(_state, key.c_str());
    lua_pushboolean(_state, static_cast<int>(value));
    const int status = lua_pcall(_state, 2, 0, NULL);
    if (status != LUA_OK)
        LERROR("Error setting value '" << key << "'. Error: " <<
                                            lua_tostring(_state, -1));
}

// character types
template <>
void ConfigurationManager::setValue(const std::string& key, const char& value) {
    lua_Integer val = static_cast<lua_Integer>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const signed char& value) {
    lua_Integer val = static_cast<lua_Integer>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const unsigned char& value) {
    lua_Unsigned val = static_cast<lua_Unsigned>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const wchar_t& value) {
    lua_Integer val = static_cast<lua_Integer>(value);
    helper::setValue(_state, key, val);
}

// integer types

template <>
void ConfigurationManager::setValue(const std::string& key, const short& value) {
    lua_Integer val = static_cast<lua_Integer>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const unsigned short& value) {
    lua_Unsigned val = static_cast<lua_Unsigned>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const int& value) {
    lua_Integer val = static_cast<lua_Integer>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const unsigned int& value) {
    lua_Unsigned val = static_cast<lua_Unsigned>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const long& value) {
    lua_Integer val = static_cast<lua_Integer>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const unsigned long& value) {
    lua_Unsigned val = static_cast<lua_Unsigned>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const long long& value) {
    lua_Integer val = static_cast<lua_Integer>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key,
                                    const unsigned long long& value)
{
    lua_Unsigned val = static_cast<lua_Unsigned>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const float& value) {
    lua_Number val = static_cast<lua_Number>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const double& value) {
    lua_Number val = static_cast<lua_Number>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const long double& value) {
    lua_Number val = static_cast<lua_Number>(value);
    helper::setValue(_state, key, val);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const std::string& value) {
    assert(_state != nullptr);

    lua_getglobal(_state, "setValue");
    lua_pushstring(_state, key.c_str());
    lua_pushstring(_state, value.c_str());
    const int status = lua_pcall(_state, 2, 0, NULL);
    if (status != LUA_OK)
        LERROR("Error setting value '" << key << "'. Error: " <<
                        lua_tostring(_state, -1));
}

void ConfigurationManager::setValue(const std::string& key, const char* value) {
    const std::string v(value);
    setValue(key, v);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::vec2& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::vec3& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
    setValue(key + "[3]", value.z);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::vec4& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
    setValue(key + "[3]", value.z);
    setValue(key + "[4]", value.w);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dvec2& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dvec3& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
    setValue(key + "[3]", value.z);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dvec4& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
    setValue(key + "[3]", value.z);
    setValue(key + "[4]", value.w);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::ivec2& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::ivec3& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
    setValue(key + "[3]", value.z);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::ivec4& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
    setValue(key + "[3]", value.z);
    setValue(key + "[4]", value.w);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::uvec2& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::uvec3& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
    setValue(key + "[3]", value.z);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::uvec4& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
    setValue(key + "[3]", value.z);
    setValue(key + "[4]", value.w);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::bvec2& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::bvec3& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
    setValue(key + "[3]", value.z);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::bvec4& value) {
    setValue(key + "[1]", value.x);
    setValue(key + "[2]", value.y);
    setValue(key + "[3]", value.z);
    setValue(key + "[4]", value.w);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::mat2x2& value) {
    const float* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::mat2x3& value) {
    const float* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::mat2x4& value) {
    const float* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::mat3x2& value) {
    const float* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::mat3x3& value) {
    const float* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
    setValue(key + "[9]", v[8]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::mat3x4& value) {
    const float* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
    setValue(key + "[9]", v[8]);
    setValue(key + "[10]", v[9]);
    setValue(key + "[11]", v[10]);
    setValue(key + "[12]", v[11]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::mat4x2& value) {
    const float* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::mat4x3& value) {
    const float* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
    setValue(key + "[9]", v[8]);
    setValue(key + "[10]", v[9]);
    setValue(key + "[11]", v[10]);
    setValue(key + "[12]", v[11]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::mat4x4& value) {
    const float* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
    setValue(key + "[9]", v[8]);
    setValue(key + "[10]", v[9]);
    setValue(key + "[11]", v[10]);
    setValue(key + "[12]", v[11]);
    setValue(key + "[13]", v[12]);
    setValue(key + "[14]", v[13]);
    setValue(key + "[15]", v[14]);
    setValue(key + "[16]", v[15]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dmat2x2& value) {
    const double* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dmat2x3& value) {
    const double* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dmat2x4& value) {
    const double* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dmat3x2& value) {
    const double* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dmat3x3& value) {
    const double* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
    setValue(key + "[9]", v[8]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dmat3x4& value) {
    const double* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
    setValue(key + "[9]", v[8]);
    setValue(key + "[10]", v[9]);
    setValue(key + "[11]", v[10]);
    setValue(key + "[12]", v[11]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dmat4x2& value) {
    const double* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dmat4x3& value) {
    const double* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
    setValue(key + "[9]", v[8]);
    setValue(key + "[10]", v[9]);
    setValue(key + "[11]", v[10]);
    setValue(key + "[12]", v[11]);
}

template <>
void ConfigurationManager::setValue(const std::string& key, const glm::dmat4x4& value) {
    const double* v = glm::value_ptr(value);
    setValue(key + "[1]", v[0]);
    setValue(key + "[2]", v[1]);
    setValue(key + "[3]", v[2]);
    setValue(key + "[4]", v[3]);
    setValue(key + "[5]", v[4]);
    setValue(key + "[6]", v[5]);
    setValue(key + "[7]", v[6]);
    setValue(key + "[8]", v[7]);
    setValue(key + "[9]", v[8]);
    setValue(key + "[10]", v[9]);
    setValue(key + "[11]", v[10]);
    setValue(key + "[12]", v[11]);
    setValue(key + "[13]", v[12]);
    setValue(key + "[14]", v[13]);
    setValue(key + "[15]", v[14]);
    setValue(key + "[16]", v[15]);
}

//
// Get
//
template <>
bool ConfigurationManager::getValue(const std::string& key, bool& value) {
    assert(_state != nullptr);

    lua_getglobal(_state, "getValue");
    lua_pushstring(_state, key.c_str());
    const int status = lua_pcall(_state, 1, 1, NULL);
    if (status != LUA_OK) {
        LERROR("Error getting value '" << key << "'. Error: " <<
                        lua_tostring(_state, -1));
        return false;
    }
    if (lua_isnil(_state, -1)) {
        lua_pop(_state, 1);
        return false;
    } else {
        const int v = lua_toboolean(_state, -1);
        value = (v != 0);
        lua_pop(_state, 1);
        return true;
    }
}

template <>
bool ConfigurationManager::getValue(const std::string& key, char& value) {
    lua_Integer val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<char>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, signed char& value) {
    lua_Integer val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<signed char>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, unsigned char& value) {
    lua_Unsigned val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<unsigned char>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, wchar_t& value) {
    lua_Integer val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<wchar_t>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, short& value) {
    lua_Integer val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<short>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, unsigned short& value) {
    lua_Unsigned val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<unsigned short>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, int& value) {
    lua_Integer val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<int>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, unsigned int& value) {
    lua_Unsigned val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<unsigned int>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, long& value) {
    lua_Integer val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<long>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, unsigned long& value) {
    lua_Unsigned val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<unsigned long>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, long long& value) {
    lua_Integer val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<long long>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, unsigned long long& value) {
    lua_Unsigned val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<unsigned long>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, float& value) {
    lua_Number val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<float>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, double& value) {
    lua_Number val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<double>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, long double& value) {
    lua_Number val;
    const bool result = helper::getValue(_state, key, val);
    if (result)
        value = static_cast<long double>(val);
    return result;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, std::string& value) {
    assert(_state != nullptr);

    lua_getglobal(_state, "getValue");
    lua_pushstring(_state, key.c_str());
    const int status = lua_pcall(_state, 1, 1, NULL);
    if (status != LUA_OK) {
        LERROR("Error getting value '" << key << "'. Error: " <<
                            lua_tostring(_state, -1));
        return false;
    }
    if (lua_isnil(_state, -1)) {
        lua_pop(_state, 1);
        return false;
    } else {
        const char* v = lua_tostring(_state, -1);
        value = std::string(v);
        lua_pop(_state, 1);
        return true;
    }
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dvec2& value) {
    lua_Number x,y;
    bool xSuccess = helper::getValue(_state, key + "[1]", x);
    bool ySuccess = helper::getValue(_state, key + "[2]", y);

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + "[\"1\"]", x);
        ySuccess = helper::getValue(_state, key + "[\"2\"]", y);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".x", x);
        ySuccess = helper::getValue(_state, key + ".y", y);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".r", x);
        ySuccess = helper::getValue(_state, key + ".g", y);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".s", x);
        ySuccess = helper::getValue(_state, key + ".t", y);
    }

    if (!(xSuccess && ySuccess))
        return false;
    else {
        value = glm::dvec2(x, y);
        return true;
    }
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dvec3& value) {
    lua_Number x,y,z;
    bool xSuccess = helper::getValue(_state, key + "[1]", x);
    bool ySuccess = helper::getValue(_state, key + "[2]", y);
    bool zSuccess = helper::getValue(_state, key + "[3]", z);

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + "[\"1\"]", x);
        ySuccess = helper::getValue(_state, key + "[\"2\"]", y);
        zSuccess = helper::getValue(_state, key + "[\"3\"]", z);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".x", x);
        ySuccess = helper::getValue(_state, key + ".y", y);
        zSuccess = helper::getValue(_state, key + ".z", z);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".r", x);
        ySuccess = helper::getValue(_state, key + ".g", y);
        zSuccess = helper::getValue(_state, key + ".b", z);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".s", x);
        ySuccess = helper::getValue(_state, key + ".t", y);
        zSuccess = helper::getValue(_state, key + ".p", z);
    }

    if (!(xSuccess && ySuccess))
        return false;
    else {
        value = glm::dvec3(x, y, z);
        return true;
    }
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dvec4& value) {
    lua_Number x,y,z,w;
    bool xSuccess = helper::getValue(_state, key + "[1]", x);
    bool ySuccess = helper::getValue(_state, key + "[2]", y);
    bool zSuccess = helper::getValue(_state, key + "[3]", z);
    bool wSuccess = helper::getValue(_state, key + "[4]", w);

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + "[\"1\"]", x);
        ySuccess = helper::getValue(_state, key + "[\"2\"]", y);
        zSuccess = helper::getValue(_state, key + "[\"3\"]", z);
        wSuccess = helper::getValue(_state, key + "[\"4\"]", w);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".x", x);
        ySuccess = helper::getValue(_state, key + ".y", y);
        zSuccess = helper::getValue(_state, key + ".z", z);
        wSuccess = helper::getValue(_state, key + ".w", w);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".r", x);
        ySuccess = helper::getValue(_state, key + ".g", y);
        zSuccess = helper::getValue(_state, key + ".b", z);
        wSuccess = helper::getValue(_state, key + ".a", w);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".s", x);
        ySuccess = helper::getValue(_state, key + ".t", y);
        zSuccess = helper::getValue(_state, key + ".p", z);
        wSuccess = helper::getValue(_state, key + ".q", w);
    }

    if (!(xSuccess && ySuccess))
        return false;
    else {
        value = glm::dvec4(x, y, z, w);
        return true;
    }
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::vec2& value) {
    glm::dvec2 v;
    const bool success = getValue(key, v);
    if (success) 
        value = glm::vec2(v);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::vec3& value) {
    glm::dvec3 v;
    const bool success = getValue(key, v);
    if (success) 
        value = glm::vec3(v);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::vec4& value) {
    glm::dvec4 v;
    const bool success = getValue(key, v);
    if (success) 
        value = glm::vec4(v);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::ivec2& value) {
    glm::dvec2 v;
    const bool success = getValue(key, v);
    if (success) 
        value = glm::ivec2(v);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::ivec3& value) {
    glm::dvec3 v;
    const bool success = getValue(key, v);
    if (success) 
        value = glm::ivec3(v);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::ivec4& value) {
    glm::dvec4 v;
    const bool success = getValue(key, v);
    if (success) 
        value = glm::ivec4(v);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::uvec2& value) {
    glm::dvec2 v;
    const bool success = getValue(key, v);
    if (success) 
        value = glm::uvec2(v);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::uvec3& value) {
    glm::dvec3 v;
    const bool success = getValue(key, v);
    if (success) 
        value = glm::uvec3(v);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::uvec4& value) {
    glm::dvec4 v;
    const bool success = getValue(key, v);
    if (success) 
        value = glm::uvec4(v);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::bvec2& value) {
    int x,y;
    bool xSuccess = helper::getValue(_state, key + "[1]", x);
    bool ySuccess = helper::getValue(_state, key + "[2]", y);

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + "[\"1\"]", x);
        ySuccess = helper::getValue(_state, key + "[\"2\"]", y);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".x", x);
        ySuccess = helper::getValue(_state, key + ".y", y);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".r", x);
        ySuccess = helper::getValue(_state, key + ".g", y);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".s", x);
        ySuccess = helper::getValue(_state, key + ".t", y);
    }

    if (!(xSuccess && ySuccess))
        return false;
    else {
        value = glm::bvec2(x != 0, y != 0);
        return true;
    }
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::bvec3& value) {
    int x,y,z;
    bool xSuccess = helper::getValue(_state, key + "[1]", x);
    bool ySuccess = helper::getValue(_state, key + "[2]", y);
    bool zSuccess = helper::getValue(_state, key + "[3]", z);

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + "[\"1\"]", x);
        ySuccess = helper::getValue(_state, key + "[\"2\"]", y);
        zSuccess = helper::getValue(_state, key + "[\"3\"]", z);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".x", x);
        ySuccess = helper::getValue(_state, key + ".y", y);
        zSuccess = helper::getValue(_state, key + ".z", z);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".r", x);
        ySuccess = helper::getValue(_state, key + ".g", y);
        zSuccess = helper::getValue(_state, key + ".b", z);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".s", x);
        ySuccess = helper::getValue(_state, key + ".t", y);
        zSuccess = helper::getValue(_state, key + ".p", z);
    }

    if (!(xSuccess && ySuccess))
        return false;
    else {
        value = glm::bvec3(x != 0, y != 0, z != 0);
        return true;
    }
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::bvec4& value) {
    int x,y,z,w;
    bool xSuccess = helper::getValue(_state, key + "[1]", x);
    bool ySuccess = helper::getValue(_state, key + "[2]", y);
    bool zSuccess = helper::getValue(_state, key + "[3]", z);
    bool wSuccess = helper::getValue(_state, key + "[4]", w);

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + "[\"1\"]", x);
        ySuccess = helper::getValue(_state, key + "[\"2\"]", y);
        zSuccess = helper::getValue(_state, key + "[\"3\"]", z);
        wSuccess = helper::getValue(_state, key + "[\"4\"]", w);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".x", x);
        ySuccess = helper::getValue(_state, key + ".y", y);
        zSuccess = helper::getValue(_state, key + ".z", z);
        wSuccess = helper::getValue(_state, key + ".w", w);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".r", x);
        ySuccess = helper::getValue(_state, key + ".g", y);
        zSuccess = helper::getValue(_state, key + ".b", z);
        wSuccess = helper::getValue(_state, key + ".a", w);
    }

    if (!(xSuccess && ySuccess)) {
        xSuccess = helper::getValue(_state, key + ".s", x);
        ySuccess = helper::getValue(_state, key + ".t", y);
        zSuccess = helper::getValue(_state, key + ".p", z);
        wSuccess = helper::getValue(_state, key + ".q", w);
    }

    if (!(xSuccess && ySuccess))
        return false;
    else {
        value = glm::bvec4(x != 0, y != 0, z != 0, w != 0);
        return true;
    }
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dmat2x2& value) {
    lua_Number values[4];
    bool success = helper::getValue(_state, key + "[1]", values[0]);
    success &= helper::getValue(_state, key + "[2]", values[1]);
    success &= helper::getValue(_state, key + "[3]", values[2]);
    success &= helper::getValue(_state, key + "[4]", values[3]);

    if (success)
        value = glm::make_mat2x2(values);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dmat2x3& value) {
    lua_Number values[6];
    bool success = helper::getValue(_state, key + "[1]", values[0]);
    success &= helper::getValue(_state, key + "[2]", values[1]);
    success &= helper::getValue(_state, key + "[3]", values[2]);
    success &= helper::getValue(_state, key + "[4]", values[3]);
    success &= helper::getValue(_state, key + "[5]", values[4]);
    success &= helper::getValue(_state, key + "[6]", values[5]);

    if (success)
        value = glm::make_mat2x3(values);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dmat2x4& value) {
    lua_Number values[8];
    bool success = helper::getValue(_state, key + "[1]", values[0]);
    success &= helper::getValue(_state, key + "[2]", values[1]);
    success &= helper::getValue(_state, key + "[3]", values[2]);
    success &= helper::getValue(_state, key + "[4]", values[3]);
    success &= helper::getValue(_state, key + "[5]", values[4]);
    success &= helper::getValue(_state, key + "[6]", values[5]);
    success &= helper::getValue(_state, key + "[7]", values[6]);
    success &= helper::getValue(_state, key + "[8]", values[7]);

    if (success)
        value = glm::make_mat2x4(values);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dmat3x2& value) {
    lua_Number values[6];
    bool success = helper::getValue(_state, key + "[1]", values[0]);
    success &= helper::getValue(_state, key + "[2]", values[1]);
    success &= helper::getValue(_state, key + "[3]", values[2]);
    success &= helper::getValue(_state, key + "[4]", values[3]);
    success &= helper::getValue(_state, key + "[5]", values[4]);
    success &= helper::getValue(_state, key + "[6]", values[5]);

    if (success)
        value = glm::make_mat3x2(values);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dmat3x3& value) {
    lua_Number values[9];
    bool success = helper::getValue(_state, key + "[1]", values[0]);
    success &= helper::getValue(_state, key + "[2]", values[1]);
    success &= helper::getValue(_state, key + "[3]", values[2]);
    success &= helper::getValue(_state, key + "[4]", values[3]);
    success &= helper::getValue(_state, key + "[5]", values[4]);
    success &= helper::getValue(_state, key + "[6]", values[5]);
    success &= helper::getValue(_state, key + "[7]", values[6]);
    success &= helper::getValue(_state, key + "[8]", values[7]);
    success &= helper::getValue(_state, key + "[9]", values[8]);

    if (success)
        value = glm::make_mat3x3(values);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dmat3x4& value) {
    lua_Number values[12];
    bool success = helper::getValue(_state, key + "[1]", values[0]);
    success &= helper::getValue(_state, key + "[2]", values[1]);
    success &= helper::getValue(_state, key + "[3]", values[2]);
    success &= helper::getValue(_state, key + "[4]", values[3]);
    success &= helper::getValue(_state, key + "[5]", values[4]);
    success &= helper::getValue(_state, key + "[6]", values[5]);
    success &= helper::getValue(_state, key + "[7]", values[6]);
    success &= helper::getValue(_state, key + "[8]", values[7]);
    success &= helper::getValue(_state, key + "[9]", values[8]);
    success &= helper::getValue(_state, key + "[10]", values[9]);
    success &= helper::getValue(_state, key + "[11]", values[10]);
    success &= helper::getValue(_state, key + "[12]", values[11]);

    if (success)
        value = glm::make_mat3x4(values);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dmat4x2& value) {
    lua_Number values[8];
    bool success = helper::getValue(_state, key + "[1]", values[0]);
    success &= helper::getValue(_state, key + "[2]", values[1]);
    success &= helper::getValue(_state, key + "[3]", values[2]);
    success &= helper::getValue(_state, key + "[4]", values[3]);
    success &= helper::getValue(_state, key + "[5]", values[4]);
    success &= helper::getValue(_state, key + "[6]", values[5]);
    success &= helper::getValue(_state, key + "[7]", values[6]);
    success &= helper::getValue(_state, key + "[8]", values[7]);

    if (success)
        value = glm::make_mat4x2(values);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dmat4x3& value) {
    lua_Number values[12];
    bool success = helper::getValue(_state, key + "[1]", values[0]);
    success &= helper::getValue(_state, key + "[2]", values[1]);
    success &= helper::getValue(_state, key + "[3]", values[2]);
    success &= helper::getValue(_state, key + "[4]", values[3]);
    success &= helper::getValue(_state, key + "[5]", values[4]);
    success &= helper::getValue(_state, key + "[6]", values[5]);
    success &= helper::getValue(_state, key + "[7]", values[6]);
    success &= helper::getValue(_state, key + "[8]", values[7]);
    success &= helper::getValue(_state, key + "[9]", values[8]);
    success &= helper::getValue(_state, key + "[10]", values[9]);
    success &= helper::getValue(_state, key + "[11]", values[10]);
    success &= helper::getValue(_state, key + "[12]", values[11]);

    if (success)
        value = glm::make_mat4x3(values);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::dmat4x4& value) {
    lua_Number values[16];
    bool success = helper::getValue(_state, key + "[1]", values[0]);
    success &= helper::getValue(_state, key + "[2]", values[1]);
    success &= helper::getValue(_state, key + "[3]", values[2]);
    success &= helper::getValue(_state, key + "[4]", values[3]);
    success &= helper::getValue(_state, key + "[5]", values[4]);
    success &= helper::getValue(_state, key + "[6]", values[5]);
    success &= helper::getValue(_state, key + "[7]", values[6]);
    success &= helper::getValue(_state, key + "[8]", values[7]);
    success &= helper::getValue(_state, key + "[9]", values[8]);
    success &= helper::getValue(_state, key + "[10]", values[9]);
    success &= helper::getValue(_state, key + "[11]", values[10]);
    success &= helper::getValue(_state, key + "[12]", values[11]);
    success &= helper::getValue(_state, key + "[13]", values[12]);
    success &= helper::getValue(_state, key + "[14]", values[13]);
    success &= helper::getValue(_state, key + "[15]", values[14]);
    success &= helper::getValue(_state, key + "[16]", values[15]);

    if (success)
        value = glm::make_mat4x4(values);
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::mat2x2& value) {
    glm::dmat2x2 v;
    const bool success = getValue(key, v);
    if (success)
        value = v;
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::mat2x3& value) {
    glm::dmat2x3 v;
    const bool success = getValue(key, v);
    if (success)
        value = v;
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::mat2x4& value) {
    glm::dmat2x4 v;
    const bool success = getValue(key, v);
    if (success)
        value = v;
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::mat3x2& value) {
    glm::dmat3x2 v;
    const bool success = getValue(key, v);
    if (success)
        value = v;
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::mat3x3& value) {
    glm::dmat3x3 v;
    const bool success = getValue(key, v);
    if (success)
        value = v;
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::mat3x4& value) {
    glm::dmat3x4 v;
    const bool success = getValue(key, v);
    if (success)
        value = v;
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::mat4x2& value) {
    glm::dmat4x2 v;
    const bool success = getValue(key, v);
    if (success)
        value = v;
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::mat4x3& value) {
    glm::dmat4x3 v;
    const bool success = getValue(key, v);
    if (success)
        value = v;
    return success;
}

template <>
bool ConfigurationManager::getValue(const std::string& key, glm::mat4x4& value) {
    glm::dmat4x4 v;
    const bool success = getValue(key, v);
    if (success)
        value = v;
    return success;
}

std::vector<std::string> ConfigurationManager::keys(const std::string& location) {
    assert(_state != nullptr);

    lua_getglobal(_state, "getKeys");
    lua_pushstring(_state, location.c_str());
    const int status = lua_pcall(_state, 1, 1, NULL);
    if (status != LUA_OK) {
        LERROR("Error getting keys from location '" << location << "'." << 
                "Error: " << lua_tostring(_state, -1));
        return std::vector<std::string>();
    }
    if (lua_isnil(_state, -1)) {
        lua_pop(_state, 1);
        return std::vector<std::string>();
    } else {
        std::vector<std::string> result;
        lua_pushnil(_state);
        while (lua_next(_state, -2) != 0) {
            result.push_back(lua_tostring(_state, -1));
            lua_pop(_state, 1);
        }
        lua_pop(_state, -1);
        return result;
    }
}

bool ConfigurationManager::hasKey(const std::string& key) {
    assert(_state != nullptr);

    lua_getglobal(_state, "hasKey");
    lua_pushstring(_state, key.c_str());
    const int status = lua_pcall(_state, 1, 1, NULL);
    if (status != LUA_OK) {
        LERROR("Error getting keys from location '" << key << "'." << 
            "Error: " << lua_tostring(_state, -1));
        return false;
    }
    const int result = lua_toboolean(_state, -1);
    return (result == 1);
}

//
// Helper
//

namespace helper {

bool getValue(lua_State* state, const std::string& key, int& value) {
    assert(state != nullptr);

    lua_getglobal(state, "getValue");
    lua_pushstring(state, key.c_str());
    const int status = lua_pcall(state, 1, 1, NULL);
    if (status != LUA_OK) {
        LERROR("Error getting value '" << key << "'. Error: " <<
            lua_tostring(state, -1));
        return false;
    }
    if (lua_isnil(state, -1)) {
        lua_pop(state, 1);
        return false;
    } else {
        value = lua_toboolean(state, -1);
        lua_pop(state, 1);
        return true;
    }
}

bool getValue(lua_State* state, const std::string& key, lua_Integer& value) {
    assert(state != nullptr);

    lua_getglobal(state, "getValue");
    lua_pushstring(state, key.c_str());
    const int status = lua_pcall(state, 1, 1, NULL);
    if (status != LUA_OK) {
        LERROR("Error getting value '" << key << "'. Error: " <<
                            lua_tostring(state, -1));
        return false;
    }
    if (lua_isnil(state, -1)) {
        lua_pop(state, 1);
        return false;
    } else {
        value = lua_tointeger(state, -1);
        lua_pop(state, 1);
        return true;
    }
}

bool getValue(lua_State* state, const std::string& key, lua_Unsigned& value) {
    assert(state != nullptr);

    lua_getglobal(state, "getValue");
    lua_pushstring(state, key.c_str());
    const int status = lua_pcall(state, 1, 1, NULL);
    if (status != LUA_OK) {
        LERROR("Error getting value '" << key << "'. Error: " <<
                            lua_tostring(state, -1));
        return false;
    }
    if (lua_isnil(state, -1)) {
        lua_pop(state, 1);
        return false;
    } else {
        value = lua_tounsigned(state, -1);
        lua_pop(state, 1);
        return true;
    }
}

bool getValue(lua_State* state, const std::string& key, lua_Number& value) {
    assert(state != nullptr);

    lua_getglobal(state, "getValue");
    lua_pushstring(state, key.c_str());
    const int status = lua_pcall(state, 1, 1, NULL);
    if (status != LUA_OK) {
        LERROR("Error getting value '" << key << "'. Error: " <<
                            lua_tostring(state, -1));
        return false;
    }
    if (lua_isnil(state, -1)) {
        lua_pop(state, 1);
        return false;
    } else {
        value = lua_tonumber(state, -1);
        lua_pop(state, 1);
        return true;
    }
}

void setValue(lua_State* state, const std::string& key, const lua_Integer& value) {
    assert(state != nullptr);

    lua_getglobal(state, "setValue");
    lua_pushstring(state, key.c_str());
    lua_pushinteger(state, value);
    const int status = lua_pcall(state, 2, 0, NULL);
    if (status != LUA_OK)
        LERROR("Error setting value '" << key << "'. Error: " <<
                            lua_tostring(state, -1));
}

void setValue(lua_State* state, const std::string& key, const lua_Unsigned& value) {
    assert(state != nullptr);

    lua_getglobal(state, "setValue");
    lua_pushstring(state, key.c_str());
    lua_pushunsigned(state, value);
    const int status = lua_pcall(state, 2, 0, NULL);
    if (status != LUA_OK)
        LERROR("Error setting value '" << key << "'. Error: " <<
                            lua_tostring(state, -1));
}

void setValue(lua_State* state, const std::string& key, const lua_Number& value) {
    assert(state != nullptr);

    lua_getglobal(state, "setValue");
    lua_pushstring(state, key.c_str());
    lua_pushnumber(state, value);
    const int status = lua_pcall(state, 2, 0, NULL);
    if (status != LUA_OK)
        LERROR("Error setting value '" << key << "'. Error: " <<
                            lua_tostring(state, -1));
}

};

} // namespace ghoul
