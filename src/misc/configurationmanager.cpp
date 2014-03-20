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

void ConfigurationManager::clear() {
    _dictionary.clear();
}

bool ConfigurationManager::loadConfiguration(const std::string& filename) {
    try {
        return lua::loadDictionary(absPath(filename), _dictionary);
    }
    catch (lua::LuaFormattingException& e) {
        LERROR("Error loading configuration from file '" << filename
                                                         << "': " << e.what());
        return false;
    }
}

std::vector<std::string> ConfigurationManager::keys(const std::string& location) {
    return _dictionary.keys(location);
}

bool ConfigurationManager::hasKey(const std::string& key) {
    return _dictionary.hasKey(key);
}

bool ConfigurationManager::setValue(const std::string& key, const char* value,
                                    bool createIntermediate) {
    const std::string v(value);
    return setValue(key, v, createIntermediate);
}

} // namespace ghoul
