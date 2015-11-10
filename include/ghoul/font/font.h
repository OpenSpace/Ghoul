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

#ifndef __FONT_H__
#define __FONT_H__

#include <ghoul/glm.h>

#include <ghoul/misc/dictionary.h>
#include <ghoul/opengl/textureatlas.h>

#include <array>
#include <map>
#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H


namespace ghoul {
    
namespace fontrendering {
    
class Font {
public:
    class Glyph {
    public:
        friend class Font;

        Glyph(wchar_t character,
              size_t width = 0,
              size_t height = 0,
              int offsetX = 0,
              int offsetY = 0,
              float advanceX = 0.f,
              float advanceY = 0.f,
              glm::vec2 texCoordTopLeft = glm::vec2(0.f),
              glm::vec2 texCoordBottomRight = glm::vec2(0.f)
        );

        float kerning(wchar_t character) const;
        
        int offsetX() const;
        int offsetY() const;
        size_t width() const;
        size_t height() const;
        
        float advanceX() const { return _advanceX; }
        float advanceY() const { return _advanceY; }
        
        const glm::vec2& texCoordTopLeft() const { return _topLeft; }
        const glm::vec2& texCoordBottomRight() const { return _bottomRight; }
        
    private:

        wchar_t _charcode; ///< Wide character this glyph represents
        
        size_t _width; ///< Glyph's width in pixels
        
        size_t _height; ///< Glyph's height in pixels
        
        int _offsetX; ///< Glyph's left bearing expressed in integer pixels
        
        /**
         * Glyphs's top bearing expressed in integer pixels.
         *
         * Remember that this is the distance from the baseline to the top-most
         * glyph scanline, upwards y coordinates being positive.
         */
        int _offsetY;
        
        /**
         * For horizontal text layouts, this is the horizontal distance (in
         * fractional pixels) used to increment the pen position when the glyph is
         * drawn as part of a string of text.
         */
        float _advanceX;
        
        /**
         * For vertical text layouts, this is the vertical distance (in fractional
         * pixels) used to increment the pen position when the glyph is drawn as
         * part of a string of text.
         */
        float _advanceY;
        
        glm::vec2 _topLeft; ///< Normalized texture coordinate of top-left corner
        glm::vec2 _bottomRight; ///< Normalized texture coordinate of bottom-right corner
        
        /// A vector of kerning pairs relative to this glyph
        std::map<wchar_t, float> _kerning;
    };
    
    
    
    Font(std::string filename, float pointSize, opengl::TextureAtlas& atlas);
    ~Font();
    
    // Needs testing
    bool operator==(const Font& rhs);
    
    bool initialize();
    
    std::string name() const;
    float pointSize() const;
    float height() const;
    
    Glyph* glyph(wchar_t character);

    size_t loadGlyphs(const std::vector<wchar_t>& glyphs);
    
    opengl::TextureAtlas& atlas();

private:
    void generateKerning();
    bool loadFace(float size, FT_Library& library, FT_Face& face);


    std::vector<Glyph*> _glyphs;
    
    opengl::TextureAtlas& _atlas;
    
    std::string _name;
    float _pointSize;
    float _height;
};
    
} // namespace fontrendering
} // namespace ghoul

#endif // __FONT_H__
