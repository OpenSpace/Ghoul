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

#include <ghoul/font/font.h>

#include <ghoul/misc/assert.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/opengl/textureatlas.h>
#include <algorithm>
#include <array>
#include <ghoul/fmt.h>
#include <ghoul/misc/misc.h>
#include <tuple>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, nullptr } };

const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

namespace {
    // Sizes in FT are given in 1/64th of pt
    constexpr const float PointConversionFactor = 64.f;
    constexpr const int DPI = 96;

    // Initializes the passed 'library' and loads the font face specified by the 'name'
    // and 'size' into the provided 'face'
    void loadFace(const std::string& name,
                  float size,
                  FT_Library& library,
                  FT_Face& face)
    {
        FT_Error error = FT_Init_FreeType(&library);
        if (error) {
            throw ghoul::fontrendering::Font::FreeTypeException(
                name, size, FT_Errors[error].code, FT_Errors[error].message
            );
        }

        // Load face
        error = FT_New_Face(library, name.c_str(), 0, &face);
        if (error) {
            FT_Done_FreeType(library);
            throw ghoul::fontrendering::Font::FreeTypeException(
                name, size, FT_Errors[error].code, FT_Errors[error].message
            );
        }

        // Select charmap
        error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
        if (error) {
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            throw ghoul::fontrendering::Font::FreeTypeException(
                name, size, FT_Errors[error].code, FT_Errors[error].message
            );
        }

        // Set char size
        error = FT_Set_Char_Size(
            face,
            static_cast<int>(size * PointConversionFactor),
            0,
            DPI,
            DPI
        );
        if (error) {
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            throw ghoul::fontrendering::Font::FreeTypeException(
                name, size, FT_Errors[error].code, FT_Errors[error].message
            );
        }
    }
} // namespace

namespace ghoul::fontrendering {

Font::FontException::FontException(const std::string& msg)
    : RuntimeError(msg, "Font")
{}

Font::GlyphException::GlyphException(std::string name, float size, wchar_t character)
    : FontException(fmt::format(
        "Glyph '{}' was not present in the font '{}' at size '{}'",
        name, size, char(character)))
    , fontName(std::move(name))
    , fontSize(size)
    , glyph(character)
{}

Font::FreeTypeException::FreeTypeException(std::string name, float size, int code,
                                           std::string msg)
    : FontException(fmt::format(
        "Error loading font '{}' for size '{}': ({}) {}", name, size, code, msg
    ))
    , fontName(std::move(name))
    , fontSize(size)
    , errorCode(code)
    , errorMessage(std::move(msg))
{}

Font::Glyph::Glyph(wchar_t character, int width, int height, float leftBearing,
                   float topBearing, float advanceX, float advanceY,
                   glm::vec2 texCoordTopLeft, glm::vec2 texCoordBottomRight,
                   glm::vec2 outlineTexCoordTopLeft, glm::vec2 outlineTexCoordBottomRight)
    : _charcode(std::move(character))
    , _width(width)
    , _height(height)
    , _leftBearing(leftBearing)
    , _topBearing(topBearing)
    , _horizontalAdvance(advanceX)
    , _verticalAdvance(advanceY)
    , _topLeft(std::move(texCoordTopLeft))
    , _bottomRight(std::move(texCoordBottomRight))
    , _outlineTopLeft(std::move(outlineTexCoordTopLeft))
    , _outlineBottomRight(std::move(outlineTexCoordBottomRight))
{}

bool Font::Glyph::operator==(const Font::Glyph& rhs) const {
    return _charcode == rhs._charcode;
}

int Font::Glyph::width() const {
    return _width;
}

int Font::Glyph::height() const {
    return _height;
}

float Font::Glyph::leftBearing() const {
    return _leftBearing;
}

float Font::Glyph::topBearing() const {
    return _topBearing;
}

float Font::Glyph::kerning(wchar_t character) const {
    auto it = _kerning.find(character);
    if (it != _kerning.end()) {
        return it->second;
    }
    else {
        return 0.f;
    }
}

float Font::Glyph::horizontalAdvance() const {
    return _horizontalAdvance;
}

float Font::Glyph::verticalAdvance() const {
    return _verticalAdvance;
}

const glm::vec2& Font::Glyph::topLeft() const {
    return _topLeft;
}

const glm::vec2& Font::Glyph::bottomRight() const {
    return _bottomRight;
}

const glm::vec2& Font::Glyph::outlineTopLeft() const {
    return _outlineTopLeft;
}

const glm::vec2& Font::Glyph::outlineBottomRight() const {
    return _outlineBottomRight;
}

Font::Font(std::string filename, float pointSize, opengl::TextureAtlas& atlas,
           Outline hasOutline, float outlineThickness)
    : _atlas(atlas)
    , _name(std::move(filename))
    , _pointSize(pointSize)
    , _height(0.f)
    , _hasOutline(hasOutline)
    , _outlineThickness(outlineThickness)
{
    ghoul_assert(!_name.empty(), "Filename must not be empty");
    ghoul_assert(_pointSize > 0.f, "Need positive point size");

    // Get font metrics at higher resolution for increased accuracy
    constexpr const float HighFaceResolutionFactor = 100.f;

    FT_Library library;
    FT_Face face;
    loadFace(_name, _pointSize * HighFaceResolutionFactor, library, face);

    _height = (face->size->metrics.height >> 6) / HighFaceResolutionFactor;

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    // -1 is a special glyph
    glyph(static_cast<wchar_t>(-1));
}

const std::string& Font::name() const {
    return _name;
}

float Font::pointSize() const {
    return _pointSize;
}

float Font::height() const {
    return _height;
}

opengl::TextureAtlas& Font::atlas() {
    return _atlas;
}

bool Font::hasOutline() const {
    return _hasOutline;
}

glm::vec2 Font::boundingBox(const std::string& text) {
    glm::vec2 result(0.f);

    const std::vector<std::string>& lines = ghoul::tokenizeString(text, '\n');

    for (const std::string& line : lines) {
        float width = 0.f;
        float height = 0.f;
        for (size_t j = 0 ; j < line.size(); ++j) {
            const Font::Glyph* const g = glyph(line[j]);
            if (g) {
                if (j > 0) {
                    width += g->kerning(line[j - 1]);
                }
                width += g->horizontalAdvance();
                height = std::max(height, static_cast<float>(g->height()));
            }
        }
        result.x = std::max(result.x, width);
        result.y += height;
    }

    result.y += (lines.size() - 1) * _height;

    return result;
}

const Font::Glyph* Font::glyph(wchar_t character) {
    // Check if charcode has been already loaded
    for (const Glyph& g : _glyphs) {
        if (g._charcode == character) {
            return &g;
        }
    }

    // charcode -1 is special: it is used for line drawing (overline, underline,
    // strikethrough) and background.
    if (character == static_cast<wchar_t>(-1)) {
        opengl::TextureAtlas::RegionHandle handle = _atlas.newRegion(4, 4);
        // The last *4 for the depth is not a danger here as _atlas.setRegion only
        // extracts as much data as is needed for the used texture atlas
        std::array<unsigned char, 4 * 4 * 4> data {};
        data.fill(std::numeric_limits<unsigned char>::max());

        _atlas.setRegionData(handle, data.data());

        Glyph glyph(static_cast<wchar_t>(-1));
        ghoul::opengl::TextureAtlas::TextureCoordinatesResult coords =
            _atlas.textureCoordinates(handle);
        glyph._topLeft = coords.topLeft;
        glyph._bottomRight = coords.bottomRight;
        _glyphs.push_back(std::move(glyph));

        return &(_glyphs.back());
    }

    // Glyph has not been already loaded
    loadGlyphs({ character });
    return &(_glyphs.back());
}

float Font::computeLeftBearing(wchar_t charcode) const {
    constexpr const float HighResolutionFactor = 10.f;
    FT_Library library = nullptr;
    FT_Face face = nullptr;
    loadFace(_name, _pointSize * HighResolutionFactor, library, face);

    FT_UInt glyphIndex = FT_Get_Char_Index(face, charcode);
    FT_Load_Glyph(face, glyphIndex, FT_LOAD_FORCE_AUTOHINT);

    FT_Glyph outlineGlyph;
    FT_Get_Glyph(face->glyph, &outlineGlyph);
    if (_hasOutline) {
        FT_Stroker stroker;
        FT_Stroker_New(library, &stroker);

        float t = _outlineThickness * HighResolutionFactor * PointConversionFactor;
        FT_Stroker_Set(
            stroker,
            static_cast<int>(t),
            FT_STROKER_LINECAP_ROUND,
            FT_STROKER_LINEJOIN_ROUND,
            0
        );

        FT_Glyph_Stroke(&outlineGlyph, stroker, 1);
        FT_Stroker_Done(stroker);
    }
    FT_Glyph_To_Bitmap(&outlineGlyph, FT_RENDER_MODE_NORMAL, nullptr, 1);

    FT_BitmapGlyph outlineBitmap = reinterpret_cast<FT_BitmapGlyph>(outlineGlyph);

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return outlineBitmap->left / HighResolutionFactor;
}

void Font::loadGlyphs(const std::vector<wchar_t>& characters) {
#define HandleError(error) \
    if (error) { \
        FT_Done_Face(face); \
        FT_Done_FreeType(library); \
        throw FreeTypeException( \
            _name, \
            _pointSize, \
            FT_Errors[error].code, \
            FT_Errors[error].message \
        ); \
    }

#define HandleErrorWithStroker(error) \
    if (error) { \
        FT_Done_Face(face); \
        FT_Done_FreeType(library); \
        FT_Stroker_Done(stroker); \
        throw FreeTypeException( \
            _name, \
            _pointSize, \
            FT_Errors[error].code, \
            FT_Errors[error].message \
        ); \
    }

    unsigned int atlasDepth  = _atlas.size().z;

    FT_Library library = nullptr;
    FT_Face face = nullptr;
    loadFace(_name, _pointSize, library, face);

    for (wchar_t charcode : characters) {
    //for (size_t ss = 0; i < characters.size(); ++i) {
        // Search through the loaded glyphs to avoid duplicates
        //wchar_t charcode = characters[i];
        auto it = std::find_if(
            _glyphs.begin(),
            _glyphs.end(),
            [charcode](const Glyph& glyph) { return glyph._charcode == charcode; }
        );
        if (it != _glyphs.end()) {
            continue;
        }

        // First generate the font without outline and store it in the font atlas
        // only if an outline is request, repeat the process for the outline

        float leftBearing = 0.f;
        float topBearing = 0.f;;
        glm::vec2 topLeft, bottomRight;
        glm::vec2 outlineTopLeft, outlineBottomRight;
        unsigned int width = 0;
        unsigned int height = 0;

        FT_UInt glyphIndex = FT_Get_Char_Index(face, charcode);
        if (glyphIndex == 0) {
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            throw FontException(fmt::format(
                "Glyph {} was not present in the FreeType face",
                char(charcode)
            ));
        }

        FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_FORCE_AUTOHINT);
        HandleError(error);

        // In case the font has an outline, we load it first as we need the size of the
        // outline for loading the base layer
        // The reason for this is that the outline is slightly bigger than the base layer
        // for most Glyphs. Therefore, if the Font has an outline, we need to increase the
        // size of the base to match the outline so that they can be rendered on top of
        // each other
        if (_hasOutline) {
            FT_Stroker stroker;
            error = FT_Stroker_New(library, &stroker);
            HandleErrorWithStroker(error);

            FT_Stroker_Set(stroker,
                static_cast<int>(_outlineThickness * PointConversionFactor),
                FT_STROKER_LINECAP_ROUND,
                FT_STROKER_LINEJOIN_ROUND,
                0
            );

            FT_Glyph outlineGlyph;
            error = FT_Get_Glyph(face->glyph, &outlineGlyph);
            HandleErrorWithStroker(error);

            error = FT_Glyph_Stroke(&outlineGlyph, stroker, 1);
            HandleErrorWithStroker(error);
            FT_Stroker_Done(stroker);

            error = FT_Glyph_To_Bitmap(&outlineGlyph, FT_RENDER_MODE_NORMAL, nullptr, 1);
            HandleError(error);

            FT_BitmapGlyph outlineBitmap = reinterpret_cast<FT_BitmapGlyph>(outlineGlyph);
            topBearing  = static_cast<float>(outlineBitmap->top);
            leftBearing = computeLeftBearing(charcode);

            width = outlineBitmap->bitmap.width / atlasDepth;
            height = outlineBitmap->bitmap.rows;

            opengl::TextureAtlas::RegionHandle handle = _atlas.newRegion(width, height);
            if (outlineBitmap->bitmap.buffer) {
                _atlas.setRegionData(handle, outlineBitmap->bitmap.buffer);
            }
            opengl::TextureAtlas::TextureCoordinatesResult res =
                _atlas.textureCoordinates(handle);
            outlineTopLeft = res.topLeft;
            outlineBottomRight = res.bottomRight;
        }

        FT_Glyph insideGlyph;
        error = FT_Get_Glyph(face->glyph, &insideGlyph);
        HandleError(error);

        error = FT_Glyph_To_Bitmap(&insideGlyph, FT_RENDER_MODE_NORMAL, nullptr, 1);
        HandleError(error);

        FT_BitmapGlyph insideBitmap = reinterpret_cast<FT_BitmapGlyph>(insideGlyph);
        topBearing = std::max(topBearing, static_cast<float>(insideBitmap->top));
        leftBearing = std::max(leftBearing, computeLeftBearing(charcode));

        // We take the maximum of the width (either 0 if there is no outline, or the
        // outline width if there is one) and the height
        width = std::max(width, insideBitmap->bitmap.width / atlasDepth);
        height = std::max(height, insideBitmap->bitmap.rows);

        opengl::TextureAtlas::RegionHandle handle = _atlas.newRegion(width, height);

        // If we don't have an outline for this font, our current 'width' and 'height'
        // corresponds to the buffer, so we can just use it straight away.
        // If we have an outline, the buffer has a different size from the region in the
        // atlas, so we need to copy the values from the buffer into the atlas region
        // first
        if (!_hasOutline) {
            if (insideBitmap->bitmap.buffer) {
                _atlas.setRegionData(handle, insideBitmap->bitmap.buffer);
            }
            ghoul::opengl::TextureAtlas::TextureCoordinatesResult res =
                _atlas.textureCoordinates(handle);
            topLeft = res.topLeft;
            bottomRight = res.bottomRight;
        }
        else {
            std::vector<unsigned char> buffer(width * height * sizeof(char), 0);
            int widthOffset = (width - insideBitmap->bitmap.width) / 2;
            int heightOffset = (height - insideBitmap->bitmap.rows) / 2;

            for (unsigned int j = 0; j < height; ++j) {
                for (unsigned int i = 0; i < width; ++i) {
                    int k = i - widthOffset;
                    int l = j - heightOffset;

                    bool inBorder =
                        (k < 0) ||
                        (k >= static_cast<int>(insideBitmap->bitmap.width)) ||
                        (l < 0) ||
                        (l >= static_cast<int>(insideBitmap->bitmap.rows));

                    if (!inBorder) {
                        buffer[(i + j*width)] =
                            insideBitmap->bitmap.buffer[k + insideBitmap->bitmap.width*l];
                    }
                }
            }

            if (!buffer.empty()) {
                _atlas.setRegionData(handle, buffer.data());
            }

            // We need to offset the texture coordinates by half of the width and height
            // differences
            opengl::TextureAtlas::TextureCoordinatesResult res =
                _atlas.textureCoordinates(
                    handle,
                    glm::ivec4(
                        widthOffset / 4.f, heightOffset / 4.f,
                        widthOffset / 4.f, heightOffset / 4.f
                    )
                );
            topLeft = res.topLeft;
            bottomRight = res.bottomRight;
        }

        // Discard hinting to get advance
        FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
        _glyphs.emplace_back(
            charcode,
            width,
            height,
            leftBearing,
            topBearing,
            face->glyph->advance.x / PointConversionFactor,
            face->glyph->advance.y / PointConversionFactor,
            topLeft,
            bottomRight,
            outlineTopLeft,
            outlineBottomRight
        );
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    _atlas.upload();
    generateKerning();
}

void Font::generateKerning() {
    FT_Library library;
    FT_Face face;

    loadFace(_name, _pointSize, library, face);

    bool hasKerning = FT_HAS_KERNING(face);
    if (!hasKerning) {
        return;
    }

    // For each combination of Glyphs, determine the kerning factors. The index starts at
    // 1 as 0 is reserved for the special background glyph
    for (size_t i = 1; i < _glyphs.size(); ++i) {
        Glyph& glyph = _glyphs[i];
        FT_UInt glyphIndex = FT_Get_Char_Index(face, glyph._charcode);
        glyph._kerning.clear();

        for (size_t j = 1; j < _glyphs.size(); ++j) {
            const Glyph& prevGlyph = _glyphs[j];
            FT_UInt prevIndex = FT_Get_Char_Index(face, prevGlyph._charcode);
            FT_Vector kerning;
            FT_Get_Kerning(face, prevIndex, glyphIndex, FT_KERNING_DEFAULT, &kerning);
            if (kerning.x != 0) {
                glyph._kerning[prevGlyph._charcode] = kerning.x / PointConversionFactor;
            }
        }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

} // namespace ghoul::fontrendering
