/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#include <ghoul/misc/assert.h>
#include <ghoul/misc/crc32.h>
#include <ghoul/opengl/shaderobject.h>

namespace ghoul::opengl {

ShaderManager::ShaderManagerError::ShaderManagerError(std::string msg)
    : RuntimeError(std::move(msg), "ShaderManager")
{}

ShaderManager& ShaderManager::ref() {
    static ShaderManager manager;
    return manager;
}

ShaderObject* ShaderManager::shaderObject(unsigned int hashedName) {
    auto it = _objects.find(hashedName);
    if (it == _objects.end()) {
        throw ShaderManagerError(
            "Could not find ShaderObject for hash '" + std::to_string(hashedName) + "'"
        );
    }
    else {
        return it->second.get();
    }
}

ShaderObject* ShaderManager::shaderObject(const std::string& name) {
    unsigned int hash = hashCRC32(name);
    try {
        return shaderObject(hash);
    }
    catch (const ShaderManagerError&) {
        // Repackage the exception as it would otherwise contain only the hash
        throw ShaderManagerError("Could not find ShaderObject for '" + name + "'");
    }
}

unsigned int ShaderManager::registerShaderObject(const std::string& name,
                                                 std::unique_ptr<ShaderObject> shader)
{
    unsigned int hashedName = hashCRC32(name);
    auto it = _objects.find(hashedName);
    if (it == _objects.end()) {
        _objects[hashedName] = std::move(shader);
        return hashedName;
    }
    else {
        throw ShaderManagerError("Name '" + name + "' was already registered");
    }
}

std::unique_ptr<ShaderObject> ShaderManager::unregisterShaderObject(
                                                                  const std::string& name)
{
    unsigned int hashedName = hashCRC32(name);
    return unregisterShaderObject(hashedName);
}

std::unique_ptr<ShaderObject> ShaderManager::unregisterShaderObject(
                                                                  unsigned int hashedName)
{
    auto it = _objects.find(hashedName);
    if (it == _objects.end()) {
        return nullptr;
    }

    std::unique_ptr<ShaderObject> tmp = std::move(it->second);
    _objects.erase(hashedName);
    return tmp;
}

unsigned int ShaderManager::hashedNameForName(const std::string& name) const {
    return hashCRC32(name);
}

} // namespace ghoul::opengl
