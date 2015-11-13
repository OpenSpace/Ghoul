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

#include <string>
#include <vector>

namespace ghoul {
namespace fontrendering {

/**
 * The Font class encapsulates a single fontface for a specific font size. It contains all
 * the information that is necessary to compute display sizes and, using the FontRendering
 * render the font to the screen. Each Font consists of Glyphs, the individual characters.
 * A Font can have an outline, which is a border of varying thickness around each
 * character. Individual Glyphs can be requested using the #glyph function, which
 * automatically loads and caches missing Glyphs on the first load. The storage backend
 * for Fonts is TextureAtlas into which all Glyphs (regular and outline) are saved. Access
 * into this TextureAtlas is performed on a per-glyph basis and each Glyph stores its
 * texture coordinates in the atlas. Each Font is uniquely identified by the combination
 * of name, font size, whether it has an outline, the thickness of the outline, and the
 * TextureAtlas it uses.
 */
class Font {
public:
    /**
     * This class contains the metrics and the texture locations in the TextureAtlas for a
     * single glyph for a specific font. Each glyph supplies two pairs of coordinates:
     * </br>
     * 1. The top left and bottom right corners of the base glyph (i.e., the regular
     * glyph if it is rendered without an outline.</br>
     * 2. The top left and bottom right corners of the outline glyph (i.e., a filled glyph
     * that can be rendered behind the base glyph in a different color to provide an
     * outline to the base.
     */
    class Glyph {
    public:
        friend class Font;

        /// The default constructor for a Glyph
        Glyph(wchar_t character,
              int width = 0,
              int height = 0,
              int offsetX = 0,
              int offsetY = 0,
              float advanceX = 0.f,
              float advanceY = 0.f,
              glm::vec2 texCoordTopLeft = glm::vec2(0.f),
              glm::vec2 texCoordBottomRight = glm::vec2(0.f),
              glm::vec2 outlineTexCoordTopLeft = glm::vec2(0.f),
              glm::vec2 outlineTexCoordBottomRight = glm::vec2(0.f)
        );
        
        bool operator==(const Glyph& rhs) const;
        
        /**
         * Returns the horizontal extent of the glyph
         * \return The horizontal extent of the glyph
         */
        int width() const;
        
        /**
         * Returns the vertical extent of the glyph
         * \return The vertical extent of the glyph
         */
        int height() const;

        /**
         * Returns the left-side bearing of the glyph
         * \return the left-side bearing of the glyph
         */
        int offsetX() const;
        
        /**
         * Returns the top-side bearing of the glyph
         * \return The top-side bearing of the glyph
         */
        int offsetY() const;

        /**
         * Returns the horizontal advance for this glyph
         * \return The horizontal advance for this glyph
         */
        float horizontalAdvance() const;
        
        /**
         * Returns the vertical advance for this glyph
         * \return The vertical advance for this glyph
         */
        float verticalAdvance() const;
        
        /**
         * Returns the kerning value between this glyph and <code>character</code>.
         * \param character The following character for which the kerning value should be
         * returned.
         * \return The kerning value between this glyph and <code>character</code>.
         */
        float kerning(wchar_t character) const;
        
        /**
         * Returns the texture coordinate that points to the top left corner of the base
         * representation for this Glyph in the TextureAtlas
         * \return The top left base texture coordinate
         */
        const glm::vec2& topLeft() const;
        
        /**
         * Returns the texture coordinate that points to the bottom right corner of the
         * base representation for this Glyph in the TextureAtlas
         * \return The bottom right base texture coordinates
         */
        const glm::vec2& bottomRight() const;
        
        /**
         * Returns the texture coordinate that points to the top left corner of the
         * outline representation for this Glyph in the TextureAtlas
         * \return The top left outline texture coordinate
         */
        const glm::vec2& outlineTopLeft() const;
        
        /**
         * Returns the texture coordinate that points to the bottom right corner of the
         * outline representation for this Glyph in the TextureAtlas
         * \return The bottom right outline texture coordinate
         */
        const glm::vec2& outlineBottomRight() const;
        
    private:
        /// The wide character that this glyph represents
        wchar_t _charcode;

        /// Glyph's width in pixels
        int _width;

        /// Glyph's height in pixels
        int _height;

        ///< Glyph's left bearing expressed in pixels
        int _offsetX;

        /// Glyphs's top bearing expressed in pixels
        int _offsetY;

        /// This is the distance used when the glyph is drawn as part
        /// of horizontal text
        float _horizontalAdvance;
        
        /// This is the distance used when the glyph is drawn as part
        /// of vertical text
        float _verticalAdvance;
        
        /// Normalized texture coordinate of top-left corner
        glm::vec2 _topLeft;
        /// Normalized texture coordinate of bottom-right corner
        glm::vec2 _bottomRight;

        /// Normalized texture coordinates for the top left of the
        /// outline
        glm::vec2 _outlineTopLeft;

        /// Normalized texture coordinates for the bottom right of the
        /// outline
        glm::vec2 _outlineBottomRight;

        /// A vector of kerning pairs relative to this glyph
        std::map<wchar_t, float> _kerning;
    };

    Font(std::string filename, float pointSize, opengl::TextureAtlas& atlas, bool outline = true, float outlineThickness = 1.f);
    
    bool operator==(const Font& rhs);
    
    bool initialize();
    
    std::string name() const;
    float pointSize() const;
    float height() const;
    
    bool outline() const;
    
    const Glyph* glyph(wchar_t character);

    size_t loadGlyphs(const std::vector<wchar_t>& glyphs);
    
    opengl::TextureAtlas& atlas();

private:
    void generateKerning();

    std::vector<Glyph> _glyphs;
    
    opengl::TextureAtlas& _atlas;
    
    std::string _name;
    float _pointSize;
    float _height;
    bool _outline;
    float _outlineThickness;
};
    
} // namespace fontrendering
} // namespace ghoul

#endif // __FONT_H__
