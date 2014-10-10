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

#include <ghoul/opengl/shadermanager.h>

#include <ghoul/opengl/shaderobject.h>
#include <ghoul/logging/logmanager.h>

#include <ghoul/misc/crc32.h>
#include <cassert>

namespace {
    const std::string _loggerCat = "ShaderManager";
}

namespace ghoul {
namespace opengl {
        
ShaderManager* ShaderManager::_manager = nullptr;
    
ShaderManager::~ShaderManager() {
    std::map<unsigned int, ShaderObject*>::iterator it = _objects.begin();
    for (; it != _objects.end(); ++it)
        delete it->second;
    _objects.clear();
}

void ShaderManager::initialize() {
    assert(_manager == nullptr);
    _manager = new ShaderManager;
    assert(_manager != nullptr);
}

void ShaderManager::deinitialize() {
    assert(_manager != nullptr);
    delete _manager;
    _manager = nullptr;
}

ShaderManager& ShaderManager::ref() {
    assert(_manager != nullptr);
    return *_manager;
}

ShaderObject* ShaderManager::shaderObject(unsigned int hashedName) {
    std::map<unsigned int, ShaderObject*>::iterator it = _objects.find(hashedName);
    if (it == _objects.end())
        return nullptr;
    else
        return it->second;
}

ShaderObject* ShaderManager::shaderObject(const std::string& name) {
    const unsigned int hash = hashCRC32(name);
    return shaderObject(hash);
}

bool ShaderManager::registerShaderObject(const std::string& name, ShaderObject* shader) {
    unsigned int hashedName = 0;
    return registerShaderObject(name, shader, hashedName);
}

bool ShaderManager::registerShaderObject(const std::string& name,
                                         ShaderObject* shader, unsigned int& hashedName)
{
    hashedName = hashCRC32(name);
    std::map<unsigned int, ShaderObject*>::iterator it = _objects.find(hashedName);
    if (it == _objects.end()) {
        _objects[hashedName] = shader;
        return true;
    }
    else {
        if (_objects[hashedName] == shader)
            LWARNING("Name '" + name + "' is already registered.");
        else
            LWARNING("Name '" + name +
                          "' is already registered for a different ShaderObject");
        return false;
    }
}

void ShaderManager::unregisterShaderObject(const std::string& name) {
    const unsigned int hashedName = hashCRC32(name);
    unregisterShaderObject(hashedName);
}

void ShaderManager::unregisterShaderObject(unsigned int hashedName) {
    std::map<unsigned int, ShaderObject*>::iterator it = _objects.find(hashedName);
    if (it != _objects.end())
        delete (it->second);
    _objects.erase(it);
}

void ShaderManager::forgetShaderObject(const std::string& name) {
    const unsigned int hashedName = hashCRC32(name);
    forgetShaderObject(hashedName);
}

void ShaderManager::forgetShaderObject(unsigned int hashedName) {
    std::map<unsigned int, ShaderObject*>::iterator it = _objects.find(hashedName);
    if (it != _objects.end())
        _objects.erase(it);
}

unsigned int ShaderManager::hashedNameForName(const std::string& name) const {
    return hashCRC32(name);
}

} // namespace opengl
} // namespace ghoul
