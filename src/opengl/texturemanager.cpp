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

#include <ghoul/opengl/texturemanager.h>

#include <ghoul/opengl/texture.h>
#include <ghoul/logging/logmanager.h>

#include <ghoul/misc/crc32.h>
#include <cassert>

namespace {
    const std::string _loggerCat = "TextureManager";
}

namespace ghoul {
namespace opengl {

TextureManager* TextureManager::_manager = nullptr;

TextureManager::TextureManager() {}
TextureManager::TextureManager(const TextureManager&){}

TextureManager::~TextureManager() {
    std::map<unsigned int, Texture*>::iterator it = _textures.begin();
    for (; it != _textures.end(); ++it)
        delete it->second;
    _textures.clear();
}

void TextureManager::initialize() {
    if (_manager == nullptr)
        _manager = new TextureManager;
    assert(_manager != nullptr);
}

void TextureManager::deinitialize() {
    delete _manager;
    _manager = nullptr;
}

TextureManager& TextureManager::ref() {
    assert(_manager != nullptr);
    return *_manager;
}

Texture* TextureManager::texture(unsigned int hashedName) {
    std::map<unsigned int, Texture*>::iterator it = _textures.find(hashedName);
    if (it == _textures.end())
        return nullptr;
    else
        return it->second;
}

Texture* TextureManager::texture(const std::string& name) {
    unsigned int hash = hashCRC32(name);
    return texture(hash);
}

bool TextureManager::registerTexture(const std::string& name, Texture* texture) {
    unsigned int hashedName = 0;
    return registerTexture(name, texture, hashedName);
}

bool TextureManager::registerTexture(const std::string& name,
                                     Texture* texture, unsigned int& hashedName)
{
    hashedName = hashCRC32(name);
    std::map<unsigned int, Texture*>::iterator it = _textures.find(hashedName);
    if (it == _textures.end()) {
        _textures[hashedName] = texture;
        return true;
    }
    else {
        if (_textures[hashedName] == texture)
            LWARNING("Name '" + name + "' is already registered in TextureManager.");
        else
            LWARNING("Name '" + name +
            "' is already registered for a different Texture in TextureManager");
        return false;
    }
}

void TextureManager::unregisterTexture(const std::string& name) {
    unsigned int hashedName = hashCRC32(name);
    unregisterTexture(hashedName);
}

void TextureManager::unregisterTexture(unsigned int hashedName) {
    std::map<unsigned int, Texture*>::iterator it = _textures.find(hashedName);
    if (it != _textures.end())
        delete (it->second);
    _textures.erase(it);
}

void TextureManager::forgetTexture(const std::string& name) {
    unsigned int hashedName = hashCRC32(name);
    forgetTexture(hashedName);
}

void TextureManager::forgetTexture(unsigned int hashedName) {
    std::map<unsigned int, Texture*>::iterator it = _textures.find(hashedName);
    if (it != _textures.end())
        _textures.erase(it);
}

unsigned int TextureManager::hashedNameForName(const std::string& name) const {
    return hashCRC32(name);
}

} // namespace opengl
} // namespace ghoul
