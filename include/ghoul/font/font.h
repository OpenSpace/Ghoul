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
    enum class Outline {
        None,
        Line,
        Inner,
        Outer
    };
    
    class Glyph {
    public:
        Glyph(wchar_t character);
        float kerning(wchar_t character);
        
//    private:

        wchar_t _charcode; ///< Wide character this glyph represents
        
        unsigned int _id; ///< Glyph id (used for display lists)
        
        size_t _width; ///< Glyph's width in pixels
        
        size_t _height; ///< Glyph's height in pixels
        
        int _offset_x; ///< Glyph's left bearing expressed in integer pixels
        
        /**
         * Glyphs's top bearing expressed in integer pixels.
         *
         * Remember that this is the distance from the baseline to the top-most
         * glyph scanline, upwards y coordinates being positive.
         */
        int _offset_y;
        
        /**
         * For horizontal text layouts, this is the horizontal distance (in
         * fractional pixels) used to increment the pen position when the glyph is
         * drawn as part of a string of text.
         */
        float _advance_x;
        
        /**
         * For vertical text layouts, this is the vertical distance (in fractional
         * pixels) used to increment the pen position when the glyph is drawn as
         * part of a string of text.
         */
        float _advance_y;
        
        glm::vec2 _topLeft; ///< Normalized texture coordinate of top-left corner
        glm::vec2 _bottomRight; ///< Normalized texture coordinate of bottom-right corner
                     //        float s0; ///< First normalized texture coordinate (x) of top-left corner
                     //
                     //        float t0; ///< Second normalized texture coordinate (y) of top-left corner
                     //
                     //        float s1; ///< First normalized texture coordinate (x) of bottom-right corner
                     //
                     //        float t1; ///< Second normalized texture coordinate (y) of bottom-right corner
        
        std::map<wchar_t, float> _kerning; ///< A vector of kerning pairs relative to this glyph
                                          // Struct kerning...
        
        Outline _outline; ///< Glyph outline type
        float _outline_thickness; ///< Glyph outline thickness
    };
    
    
    Font(std::string filename, float pointSize, opengl::TextureAtlas& atlas);
    ~Font();
    
    bool initialize();
    
    Glyph* glyph(wchar_t character);
    
    std::string name() const;
    float fontSize() const;
    
    size_t loadGlyphs(const std::vector<wchar_t>& glyphs);

private:
    void generateKerning();
    bool loadFace(float size, FT_Library& library, FT_Face& face);
//    bool getFace(FT_Library* library, FT_Face* face);
//    bool getFace(float size, FT_Library* library, FT_Face* face);
//    bool getHiResFace(FT_Library* library, FT_Face* face);



    std::vector<Glyph*> _glyphs;
    
    opengl::TextureAtlas& _atlas;
    std::string _name;
    float _pointSize;
    bool _autoHinting;
    Outline _outlineType;
    float _outlineThickness;
    bool _lcdFiltering;
    bool _kerning;
    std::array<unsigned char, 5> _lcdWeights;
    float _height;
    float _linegap;
    float _ascender;
    float _decender;
    float _underlinePosition;
    float _underlineThickness;
    
    
};
    
} // namespace fontrendering
} // namespace ghoul

#endif // __FONT_H__
