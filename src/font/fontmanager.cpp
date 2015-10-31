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

namespace {
    const std::string _loggerCat = "FontManager";
}

namespace ghoul {
namespace fontrendering {

FontManager::FontManager(glm::ivec3 atlasDimensions)
    : _textureAtlas(atlasDimensions.x, atlasDimensions.y, atlasDimensions.z)
{
    _defaultCharacterSet = { L' ' };
}
    
FontManager::FontManager(const FontManager& rhs)
    : _textureAtlas(rhs._textureAtlas.width(), rhs._textureAtlas.height(), rhs._textureAtlas.depth())
{
    ghoul_assert(false, "unimplemented");
}
    
FontManager::FontManager(FontManager&& rhs)
    : _textureAtlas(rhs._textureAtlas)
{
    ghoul_assert(false, "unimplemented");
}
    
FontManager::~FontManager() {
    for (Font* f : _fonts)
        delete f;
}
    
FontManager& FontManager::operator=(const FontManager &rhs) {
    ghoul_assert(false, "unimplemented");
    
    return *this;
}
    
FontManager& FontManager::operator=(FontManager&& rhs) {
    ghoul_assert(false, "unimplemented");
    return *this;
}
    
Font* FontManager::fontFromFilename(const std::string& filename, float fontSize) {
    auto it = std::find_if(_fonts.begin(),
                 _fonts.end(),
                 [filename, fontSize](Font* f) { return f->name() == filename && f->fontSize() == fontSize; }
                 );
    
    if (it != _fonts.end())
        return *it;
    
    Font* f = new Font(filename, fontSize, _textureAtlas);
    if (f == nullptr) {
        LERROR("Error loading font with file '" << filename << "' and size '" << fontSize << "'");
        return nullptr;
    }
    
    f->loadGlyphs(_defaultCharacterSet);
    _fonts.push_back(f);
}
    
    
} // namespace fontrendering
} // namespace ghoul
