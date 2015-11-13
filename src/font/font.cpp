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

#include <ghoul/font/font.h>

#include <ghoul/misc/assert.h>
#include <ghoul/logging/logmanager.h>

#include <algorithm>
#include <array>

#include <cstdarg>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H



#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

#ifdef WIN32
#define vscprintf(f,a) _vscprintf(f,a)
#else
static int vscprintf(const char* format, va_list pargs) {
    va_list argcopy;
    va_copy(argcopy, pargs);
    int retval = vsnprintf(nullptr, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}
#endif

namespace {
    const std::string _loggerCat = "Font";
    const float PointConversionFactor = 64.f;  // Sizes in FT are given in 1/64th of pt
    const int DPI = 72;

    // Initializes the passed 'library' and loads the font face specified by the 'name'
    // and 'size' into the provided 'face'
    bool loadFace(const std::string& name,
                  float size,
                  FT_Library& library,
                  FT_Face& face)
    {
        FT_Error error = FT_Init_FreeType(&library);
        if (error) {
            LERROR("FT_Error: " <<
                   FT_Errors[error].code <<
                   " (" << FT_Errors[error].message << ")");
            return false;
        }
        
        // Load face
        error = FT_New_Face(library, name.c_str(), 0, &face);
        if (error) {
            LERROR("FT_Error: " <<
                   FT_Errors[error].code <<
                   " (" << FT_Errors[error].message << ")");
            FT_Done_FreeType(library);
            return false;
        }
        
        // Select charmap
        error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
        if (error) {
            LERROR("FT_Error: " <<
                   FT_Errors[error].code <<
                   " (" << FT_Errors[error].message << ")");
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            return false;
        }
        
        // Set char size
        error = FT_Set_Char_Size(face, (int)(size * PointConversionFactor), 0, DPI , DPI);
        if (error) {
            LERROR("FT_Error: " <<
                   FT_Errors[error].code <<
                   " (" << FT_Errors[error].message << ")");
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            return false;
        }
        return true;
    }
}


namespace ghoul {
namespace fontrendering {
    
Font::Glyph::Glyph(wchar_t character,
                   int width,
                   int height,
                   int offsetX,
                   int offsetY,
                   float advanceX,
                   float advanceY,
                   glm::vec2 texCoordTopLeft,
                   glm::vec2 texCoordBottomRight,
                   glm::vec2 outlineTexCoordTopLeft,
                   glm::vec2 outlineTexCoordBottomRight)
    : _charcode(std::move(character))
    , _width(width)
    , _height(height)
    , _offsetX(offsetX)
    , _offsetY(offsetY)
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

int Font::Glyph::offsetX() const {
    return _offsetX;
}

int Font::Glyph::offsetY() const {
    return _offsetY;
}
    
float Font::Glyph::kerning(wchar_t character) const {
    auto it = _kerning.find(character);
    if (it != _kerning.end())
        return it->second;
    else
        return 0.f;
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
   
Font::Font(std::string filename,
           float pointSize,
           opengl::TextureAtlas& atlas,
           bool hasOutline,
           float outlineThickness)
    : _atlas(atlas)
    , _name(std::move(filename))
    , _pointSize(pointSize)
    , _height(0.f)
    , _hasOutline(hasOutline)
    , _outlineThickness(outlineThickness)
{
    ghoul_assert(_pointSize > 0.f, "Need positive point size");
    ghoul_assert(!_name.empty(), "Empty file name not allowed");
}

bool Font::initialize() {
    // Get font metrics at higher resolution for increased accuracy
    static const float HighFaceResolutionFactor = 100.f;

    FT_Library library;
    FT_Face face;

    bool success = loadFace(_name, _pointSize * HighFaceResolutionFactor, library, face);
    if (!success)
        return false;
    
    _height = (face->size->metrics.height >> 6) / HighFaceResolutionFactor;
    
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    
    // -1 is a special glyph
    glyph(-1);
    
    return true;
}

std::string Font::name() const {
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

const Font::Glyph* Font::glyph(wchar_t character) {
    using TextureAtlas = opengl::TextureAtlas;
    
    // Check if charcode has been already loaded
    for (const Glyph& g : _glyphs) {
        if (g._charcode == character)
            return &g;
    }
    
    // charcode -1 is special: it is used for line drawing (overline, underline,
    // strikethrough) and background.
    if (character == static_cast<wchar_t>(-1)) {
        TextureAtlas::RegionHandle handle = _atlas.newRegion(4, 4);
        if (handle == TextureAtlas::InvalidRegion)
            return nullptr;
        
        // The last *4 for the depth is not a danger here as _atlas.setRegion only
        // extracts as much data as is needed for the used texture atlas
        std::array<unsigned char, 4*4*4> data;
        data.fill(std::numeric_limits<unsigned char>::max());
        
        _atlas.setRegionData(handle, data.data());
        
        Glyph glyph(static_cast<wchar_t>(-1));
        _atlas.getTextureCoordinates(handle, glyph._topLeft, glyph._bottomRight);
        _glyphs.push_back(std::move(glyph));
        
        return &(_glyphs.back());
    }
    
    // Glyph has not been already loaded
    size_t nGlyphNotLoaded = loadGlyphs({character});
    if (nGlyphNotLoaded == 0)
        return &(_glyphs.back());
    else {
        LERROR("Glyphs '" << character << "' could not be loaded");
        return nullptr;
    }
}
    
size_t Font::loadGlyphs(const std::vector<wchar_t>& characters) {
    using TextureAtlas = opengl::TextureAtlas;
    
    size_t missed = 0;
    
    unsigned int atlasDepth  = _atlas.size().z;
    
    FT_Library library;
    FT_Face face;
    bool success = loadFace(_name, _pointSize, library, face);
    if (!success)
        return characters.size();

    for (size_t i = 0; i < characters.size(); ++i) {
        
        // Search through the loaded glyphs to avoid duplicates
        wchar_t charcode = characters[i];
        auto it = std::find_if(
            _glyphs.begin(),
            _glyphs.end(),
            [charcode](const Glyph& glyph) { return glyph._charcode == charcode; }
        );
        if (it != _glyphs.end())
            continue;
        
        // First generate the font without outline and store it in the font atlas
        // only if an outline is request, repeat the process for the outline
        int ft_glyph_top, ft_glyph_left;
        glm::vec2 topLeft, bottomRight;
        glm::vec2 outlineTopLeft, outlineBottomRight;
        unsigned int w = 0, h = 0;
        
        FT_Int32 flags = 0;
        flags |= FT_LOAD_FORCE_AUTOHINT;
        
        FT_UInt glyphIndex = FT_Get_Char_Index(face, characters[i]);
        if (glyphIndex == 0) {
            LERROR("Glyph was not present in the FreeType face");
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            return characters.size() - i;
        }
        
        if (_hasOutline) {
            FT_Int32 outlineFlags = flags;
            //            flags |= FT_LOAD_NO_BITMAP;
            
            FT_Error error = FT_Load_Glyph(face, glyphIndex, outlineFlags);
            if (error) {
                LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                FT_Done_Face(face);
                FT_Done_FreeType(library);
                return characters.size() - i;
            }
            
            
            FT_Stroker stroker;
            error = FT_Stroker_New(library, &stroker);
            if (error) {
                LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return characters.size() - i;
            }
            
            FT_Stroker_Set(stroker,
                           static_cast<int>(_outlineThickness * PointConversionFactor),
                           //                           static_cast<int>(OutlineThickness * HighResolution),
                           FT_STROKER_LINECAP_ROUND,
                           FT_STROKER_LINEJOIN_ROUND,
                           0);
            
            FT_Glyph outlineGlyph;
            error = FT_Get_Glyph(face->glyph, &outlineGlyph);
            if (error) {
                LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return characters.size() - i;
            }
//            error = FT_Glyph_StrokeBorder( &outlineGlyph., stroker, 0, 1 );
          
            error = FT_Glyph_Stroke(&outlineGlyph, stroker, 1);
            //            error = FT_Glyph_StrokeBorder(&outlineGlyph, stroker, false, true);
            if (error) {
                LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return characters.size() - i;
            }
            
            error = FT_Glyph_To_Bitmap(&outlineGlyph, FT_RENDER_MODE_NORMAL, 0, 1);
            if (error) {
                LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return characters.size() - i;
            }
            FT_BitmapGlyph outlineBitmap;
            outlineBitmap = (FT_BitmapGlyph)outlineGlyph;
            
            ft_glyph_top    = outlineBitmap->top;
            ft_glyph_left   = outlineBitmap->left;
            FT_Stroker_Done(stroker);
            
            // We want each glyph to be separated by at least one black pixel
            // (for example for shader used in demo-subpixel.c)
            w = outlineBitmap->bitmap.width/atlasDepth;
            h = outlineBitmap->bitmap.rows;
            
            TextureAtlas::RegionHandle handle = _atlas.newRegion(w, h);
            if (handle == TextureAtlas::InvalidRegion) {
                missed++;
                LERROR("Texture atlas is full");
                continue;
            }
            _atlas.setRegionData(handle, outlineBitmap->bitmap.buffer);
            _atlas.getTextureCoordinates(handle, outlineTopLeft, outlineBottomRight);
        }
        
        FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_FORCE_AUTOHINT);
        if (error) {
            LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            return characters.size() - i;
        }
        
        FT_Glyph insideGlyph;
        error = FT_Get_Glyph(face->glyph, &insideGlyph);
        if (error) {
            LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            return characters.size() - i;
        }
        
        error = FT_Glyph_To_Bitmap(&insideGlyph, FT_RENDER_MODE_NORMAL, nullptr, true);
        if (error) {
            LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            return characters.size() - i;
        }
        
        
        FT_BitmapGlyph insideBitmap = (FT_BitmapGlyph)insideGlyph;
        
        ft_glyph_top = insideBitmap->top;
        ft_glyph_left = insideBitmap->left;
        
        w = std::max(w, insideBitmap->bitmap.width/atlasDepth);
        h = std::max(h, insideBitmap->bitmap.rows);
        
        
        TextureAtlas::RegionHandle handle = _atlas.newRegion(w, h);
        if (handle == TextureAtlas::InvalidRegion) {
            missed++;
            LERROR("Texture atlas is full");
            continue;
        }
       
        if (_hasOutline) {
            std::vector<unsigned char> buffer(w * h * sizeof(char), 0);
            int widthOffset = w - insideBitmap->bitmap.width;
            int heightOffset = h - insideBitmap->bitmap.rows;
            
            int k, l;
            for (unsigned int j = 0; j < h; ++j) {
                for (unsigned int i = 0; i < w; ++i) {
                    k = i - widthOffset;
                    l = j - heightOffset;
                    buffer[(i + j*w)] =
                    (k >= static_cast<int>(insideBitmap->bitmap.width) || l >= static_cast<int>(insideBitmap->bitmap.rows) || k < 0 || l < 0) ?
                    0 : insideBitmap->bitmap.buffer[k + insideBitmap->bitmap.width*l];
                    
                }
            }
            
            _atlas.setRegionData(handle, buffer.data());
            
            _atlas.getTextureCoordinates(
                handle,
                topLeft,
                bottomRight,
                glm::ivec4(widthOffset / 2.f, heightOffset / 2.f, 0, 0)
            );
        }
        else {
            _atlas.setRegionData(handle, insideBitmap->bitmap.buffer);
            _atlas.getTextureCoordinates(handle, topLeft, bottomRight);
        }

        // Discard hinting to get advance
        FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
        
        _glyphs.emplace_back(
            characters[i],
            w,
            h,
            ft_glyph_left,
            ft_glyph_top,
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
    return missed;
}
    
void Font::generateKerning() {
    FT_Library library;
    FT_Face face;
    
    bool success = loadFace(_name, _pointSize, library, face);
    if (!success)
        return;
    
    /* For each glyph couple combination, check if kerning is necessary */
    /* Starts at index 1 since 0 is for the special backgroudn glyph */
    for (size_t i = 1; i < _glyphs.size(); ++i) {
        Glyph& glyph = _glyphs[i];
        FT_UInt glyphIndex = FT_Get_Char_Index(face, glyph._charcode);
        glyph._kerning.clear();
        
        for (size_t j = 1; j < _glyphs.size(); ++j) {
            const Glyph& prevGlyph = _glyphs[j];
            FT_UInt prevIndex = FT_Get_Char_Index(face, prevGlyph._charcode);
            FT_Vector kerning;
            FT_Get_Kerning(face, prevIndex, glyphIndex, FT_KERNING_UNFITTED, &kerning);
            if (kerning.x != 0) {
                glyph._kerning[prevGlyph._charcode] = kerning.x / (PointConversionFactor*PointConversionFactor);
            }
        }
    }
    
    FT_Done_Face( face );
    FT_Done_FreeType( library );
}

} // namespace fontrendering
} // namespace ghoul
