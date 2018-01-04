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

#ifndef __GHOUL___FONT___H__
#define __GHOUL___FONT___H__

#include <ghoul/glm.h>
#include <ghoul/misc/boolean.h>
#include <ghoul/misc/exception.h>
#include <ghoul/opengl/textureatlas.h>

#include <map>
#include <string>
#include <vector>

namespace ghoul::fontrendering {

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
    using Outline = ghoul::Boolean;

    /// Superclass for all font-related exceptions that this class throws
    struct FontException : public RuntimeError {
        explicit FontException(const std::string& msg);
    };

    /// The exception that gets thrown if an error occurs in the Font handling
    struct GlyphException : public FontException {
        explicit GlyphException(std::string name, float size, wchar_t character);

        /// The name of the font for which the glyph was missing
        std::string fontName;

        /// The font size of the font for which the glyph was missing
        float fontSize;

        /// The glyph that was missing from the font
        wchar_t glyph;
    };

    /// The exception that gets thrown when a FreeType-specific error occurs
    struct FreeTypeException : public FontException {
        explicit FreeTypeException(std::string name, float size, int code,
            std::string message);

        /// The name of the font for which the FreeType error occurred
        std::string fontName;

        /// The size of the font for which the FreeType error occurred
        float fontSize;

        /// The FreeType error code that occurred
        int errorCode;

        /// The FreeType error message that occurred
        std::string errorMessage;
    };

    /**
     * This class contains the metrics and the texture locations in the TextureAtlas for a
     * single glyph for a specific font. Each glyph supplies two pairs of coordinates:
     * <br>
     * 1. The top left and bottom right corners of the base glyph (i.e., the regular
     * glyph if it is rendered without an outline.<br>
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
              float leftBearing = 0.f,
              float topBearing = 0.f,
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
        float leftBearing() const;

        /**
         * Returns the top-side bearing of the glyph
         * \return The top-side bearing of the glyph
         */
        float topBearing() const;

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
         * Returns the kerning value between this glyph and \p character.
         * \param character The following character for which the kerning value should be
         * returned.
         * \return The kerning value between this glyph and \p character
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
        float _leftBearing;

        /// Glyphs's top bearing expressed in pixels
        float _topBearing;

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

    /**
     * Constructor creating a new Font with the specified \p filename at the provided
     * \p pointSize. The Glyphs of this Font will be stored in the \p atlas TextureAtlas
     * if there is enough free space. If \p outline is <code>true</code> two sets of
     * Glyphs are created which are combined to provide an outline of thickness
     * \p outlineThickness to the glyphs.
     * \param filename The full path to the font file
     * \param pointSize The font size in pt
     * \param atlas The TextureAtlas which holds the created Glyphs
     * \param hasOutline A flag whether Glyphs of this Font should have an outline or not
     * \param outlineThickness The thickness of the outline. This setting is ignored if
     * the Font does not have an outline
     * \throws FreeTypeException If there was an error loading the basic font information
     * \pre \p filename must not be empty
     * \pre \p pointSize must be positive and bigger than 0
     */
    Font(std::string filename,
         float pointSize,
         opengl::TextureAtlas& atlas,
         Outline hasOutline = Outline::Yes,
         float outlineThickness = 1.f
    );

    /**
     * Returns the name of the Font
     * \return The name of the Font
     */
    std::string name() const;

    /**
     * Returns the font size of this Font
     * \return The font size of this Font
     */
    float pointSize() const;

    /**
     * Returns the line seperator for this Font. This is the vertical length that
     * separates two consecutive lines.
     * \return The vertical line separation
     */
    float height() const;

    /**
     * Returns whether this Font has an outline or not
     * \return <code>true</code> if this Font has an outline, <code>false</code> otherwise
     */
    bool hasOutline() const;

    /**
     * Computes and retures the bounding box for the passed string with the settings of
     * this Font. The value returned is in pixel values and provides the width and the
     * height of the text if it were to be rendered.
     * \param text The format text that is rendered to the screen. This text can contain
     * symbolic constants (the same as in printf) to refer to later variable arguments,
     * which are substituted. The <code>text</code> can also contain '\\n' to have a
     * linebreak, which is of the correct length with regard to the selected font. This
     * parameter cannot be a <code>nullptr</code>.
     * \return The pixel coordinates of the bounding box of the passed text
     * \pre \p text must not be empty
     */
    glm::vec2 boundingBox(const char* text, ...);

    /**
     * Returns the Glyph that representes the passed \p character. The first call to this
     * function for each character creates and caches the Glyph before returning it.
     * \param character The character for which the Glyph should be returned
     * \return A pointer to the Glyph
     * \throw FreeTypeException If a FreeType exception occurred while loading the glyph
     * \throw GlyphException If there was an error loading the glyph
     */
    const Glyph* glyph(wchar_t character);

    /**
     * Preloads a list of Glyphs. Glyphs that are passed as part of  \p characters that
     * have been loaded previously are ignored and not loaded multiple times.
     * \param characters A list of characters for which Glyphs should be created and
     * cached
     * \throw FreeTypeException If a FreeType exception occurred while loading the glyphs
     * \throw GlyphException If there was an error loading the glyph
     */
    void loadGlyphs(const std::vector<wchar_t>& characters);

    /**
     * Returns the TextureAtlas that stores all of the Glyphs for this Font
     * \return The TextureAtlas that stores all of the Glyphs for this Font
     */
    opengl::TextureAtlas& atlas();

private:
    /// Generates the Kerning values for all Glyph pairs that have sofar been loaded
    void generateKerning();

    /// Computes the left bearing for a glyph with a higher accuracy
    float computeLeftBearing(wchar_t charcode) const;

    /// A list of all loaded Glyphs
    std::vector<Glyph> _glyphs;

    /// The TextureAtlas backend storage for the loaded Glyphs
    opengl::TextureAtlas& _atlas;

    /// The file name of this Font
    std::string _name;

    /// The font size in pt
    float _pointSize;

    /// The vertical distance between two consecutive lines
    float _height;

    /// Whether this Font has an outline or not
    Outline _hasOutline;

    /// The thickness of the outline
    float _outlineThickness;
};

} // namespace ghoul::fontrendering

#endif // __GHOUL___FONT___H__
