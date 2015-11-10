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

#ifndef __FONTMANAGER_H__
#define __FONTMANAGER_H__

#include <ghoul/font/font.h>
#include <ghoul/glm.h>
#include <ghoul/opengl/textureatlas.h>

#include <ghoul/misc/dictionary.h>

#include <map>
#include <string>
#include <vector>

namespace ghoul {
namespace fontrendering {
    
class FontManager {
public:
    
    FontManager(glm::ivec3 atlasDimensions = glm::ivec3(512, 512, 1));
    FontManager(const FontManager& rhs);
    FontManager(FontManager&& rhs);
    FontManager& operator=(const FontManager& rhs);
    FontManager& operator=(FontManager&& rhs);
    ~FontManager();
    
    // Does not check whether the file exists
    // Stores the full path internally
    bool registerFontPath(const std::string& fontName, const std::string& filePath);
    
    Font* font(const std::string& name, float fontSize);
    
private:
    ghoul::opengl::TextureAtlas _textureAtlas;
    std::map<std::string, Font*> _fonts;
    std::map<std::string, std::string> _fontPaths;
    std::vector<wchar_t> _defaultCharacterSet;
};
    
} // namespace fontrendering
} // namespace ghoul

#endif // __FONTMANAGER_H__
