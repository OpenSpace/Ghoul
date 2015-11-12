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

#include <ghoul/font/fontmanager.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/crc32.h>


namespace {
    const std::string _loggerCat = "FontManager";
}

namespace ghoul {
namespace fontrendering {
    
FontManager::FontManager(glm::ivec3 atlasDimensions)
    : _textureAtlas(std::move(atlasDimensions))
    , _defaultCharacterSet({
        L' ', L'!', L'\\', L'"', L'#', L'$', L'%', L'&', L'\'', L'(',
        L')', L'*', L'+', L',', L'-', L'.', L'/', L'0', L'1', L'2',
        L'3', L'4', L'5', L'6', L'7', L'8', L'9', L':', L';', L'<',
        L'=', L'>', L'?', L'@', L'A', L'B', L'C', L'D', L'E', L'F',
        L'G', L'H', L'I', L'J', L'K', L'L', L'M', L'N', L'O', L'P',
        L'Q', L'R', L'S', L'T', L'U', L'V', L'W', L'X', L'Y', L'Z',
        L'[', L']', L'^', L'_', L'~', L'a', L'b', L'c', L'd', L'e',
        L'f', L'g', L'h', L'i', L'j', L'k', L'l', L'm', L'n', L'o',
        L'p', L'q', L'r', L's', L't', L'u', L'v', L'w', L'x', L'y',
        L'z', L'{', L'|', L'}'
    })
{}
    
FontManager::FontManager(const FontManager& rhs)
    : _textureAtlas(rhs._textureAtlas)
    , _fontPaths(rhs._fontPaths)
    , _defaultCharacterSet(rhs._defaultCharacterSet)
{
    // TODO: This has to be tested ---abock
    for (const auto& font : rhs._fonts)
        _fonts.emplace(font.first, new Font(*(font.second)));
}
    
FontManager::FontManager(FontManager&& rhs)
    : _textureAtlas(std::move(rhs._textureAtlas))
    , _fonts(std::move(rhs._fonts))
    , _fontPaths(std::move(rhs._fontPaths))
    , _defaultCharacterSet(std::move(rhs._defaultCharacterSet))
{}
    
FontManager::~FontManager() {
    for (auto f : _fonts)
        delete f.second;
}
    
FontManager& FontManager::operator=(const FontManager& rhs) {
    if (this != &rhs) {
        _textureAtlas = rhs._textureAtlas;
        // TODO: This has to be tested ---abock
        for (const auto& font : rhs._fonts)
            _fonts.emplace(font.first, new Font(*(font.second)));
        _fontPaths = rhs._fontPaths;
        _defaultCharacterSet = rhs._defaultCharacterSet;


    }
    return *this;
}
    
FontManager& FontManager::operator=(FontManager&& rhs) {
    if (this != &rhs) {
        _textureAtlas = std::move(rhs._textureAtlas);
        _fonts = std::move(rhs._fonts);
        _fontPaths = std::move(rhs._fontPaths);
        _defaultCharacterSet = std::move(rhs._defaultCharacterSet);
        
    }
    return *this;
}
    
bool FontManager::registerFontPath(const std::string& fontName,
                                   const std::string& filePath)
{
    unsigned int hash;
    return registerFontPath(fontName, filePath, hash);
}

bool FontManager::registerFontPath(const std::string& fontName,
                                   const std::string& filePath,
                                   unsigned int& hashedName)
{
    unsigned int hash = hashCRC32(fontName);
    auto it = _fontPaths.find(hash);
    if (it != _fontPaths.end()) {
        const std::string& registeredPath = it->second;
        
        if (registeredPath == filePath) {
            hashedName = hash;
            return true;
        }
        else {
            LERROR("Font '" << fontName << "' was registered with path '" <<
                   registeredPath << "' before. Trying to register with path '" <<
                   filePath << "' now");
            return false;
        }
    }
    _fontPaths[hash] = filePath;
    hashedName = hash;
    return true;
}
    
Font* FontManager::font(const std::string& name, float fontSize, bool loadGlyphs) {
    unsigned int hash = hashCRC32(name);
    Font* res = font(hash, fontSize, loadGlyphs);
    if (res == nullptr)
        LERROR("Loading of font '" << name << "' failed");
    return res;
}
    
Font* FontManager::font(unsigned int hashName, float fontSize, bool loadGlyphs) {
    auto itPath = _fontPaths.find(hashName);
    if (itPath == _fontPaths.end()) {
        LERROR("Font with hash '" << hashName << "' is not a registered font");
        return nullptr;
    }
    
    auto fonts = _fonts.equal_range(hashName);
    for (auto it = fonts.first; it != fonts.second; ++it) {
        if (it->second->pointSize() == fontSize)
            return it->second;
    }
    
    
    Font* f = new Font(_fontPaths[hashName], fontSize, _textureAtlas);
    
    bool initSuccess = f->initialize();
    if (!initSuccess ) {
        delete f;
        return nullptr;
    }
    
    if (loadGlyphs) {
        size_t nFailedGlyphs = f->loadGlyphs(_defaultCharacterSet);
        if (nFailedGlyphs != 0) {
            delete f;
            return nullptr;
        }
    }
    
    _fonts.emplace(hashName, f);
    return f;
}
    
   
} // namespace fontrendering
} // namespace ghoul
