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

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

//Replacement for Visual Studio's _vscprintf function
#if (_MSC_VER < 1400) //if older than visual studio 2005
static int vscprintf (const char * format, va_list pargs)
{
    int retval;
    va_list argcopy;
    va_copy(argcopy, pargs);
    retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}
#else
#define vscprintf(f,a) _vscprintf(f,a)
#endif


namespace {
    const std::string _loggerCat = "Font";
    const float HighResolution = 64.f;
    
    const float HighFaceResolutionFactor = 100.f;
}

namespace ghoul {
namespace fontrendering {
    
Font::Glyph::Glyph(wchar_t character)
    : _charcode(std::move(character))
{
    
}
    
float Font::Glyph::kerning(wchar_t character) {
    auto it = _kerning.find(character);
    if (it != _kerning.end())
        return it->second;
    else
        return 0.f;
}
    
Font::Font(std::string filename, float pointSize, opengl::TextureAtlas& atlas)
    : _atlas(atlas)
    , _name(std::move(filename))
    , _pointSize(pointSize)
    , _autoHinting(true)
    , _outlineType(Outline::None)
    , _outlineThickness(0.f)
    , _lcdFiltering(true)
    , _kerning(true)
    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    , _lcdWeights({{0x10, 0x40, 0x70, 0x40, 0x10}})
    , _height(0.f)
    , _linegap(0.f)
    , _ascender(0.f)
    , _decender(0.f)
    , _underlinePosition(0.f)
    , _underlineThickness(0.f)
{
    ghoul_assert(_pointSize > 0.f, "Need positive point size");
    ghoul_assert(!_name.empty(), "Empty file name not allowed");
}
    
Font::~Font() {
        
}
    
bool Font::initialize() {
    FT_Library library;
    FT_Face face;

    // Get font metrics at high resolution
    bool success = loadFace(_pointSize * HighFaceResolutionFactor, library, face);
    if (!success)
        return false;
    
    _underlinePosition = face->underline_position / (HighResolution * HighResolution) * _pointSize;
    _underlinePosition = std::round(_underlinePosition);
    if (_underlinePosition > -2.f)
        _underlinePosition = -2.f;
//    _underlinePosition = std::max(-2.f, _underlinePosition);

    _underlineThickness = face->underline_thickness / (HighResolution * HighResolution) * _pointSize;
    _underlineThickness = std::round(_underlineThickness);
    if (_underlineThickness < 1.f)
        _underlineThickness = 1.f;
//    _underlineThickness = std::min(1.f, _underlineThickness);

    
    FT_Size_Metrics metrics = face->size->metrics;
    _ascender = (metrics.ascender >> 6) / 100.f;
    _decender = (metrics.descender >> 6) / 100.f;
    _height = (metrics.height >> 6) / 100.f;
    _linegap = _height - _ascender + _decender;
    
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    
    /* -1 is a special glyph */
    
    glyph(-1);
//    texture_font_get_glyph(self, -1 );
    
    return true;
}
    
Font::Glyph* Font::glyph(wchar_t character) {
    /* Check if charcode has been already loaded */
    for (size_t i = 0; i < _glyphs.size(); ++i) {
        Glyph* glyph = _glyphs[i];
        
        bool correctCharacter = (glyph->_charcode == character);
        bool isSpecialCharacter = (character == static_cast<wchar_t>(-1));
        bool correctOutlineType = glyph->_outline == _outlineType;
        bool correctOutlineThickness = glyph->_outline_thickness == _outlineThickness;
        

        if (correctCharacter && (isSpecialCharacter || (correctOutlineType &&
             correctOutlineThickness)))
        {
            return glyph;
        }
        
    }
    
    /* charcode -1 is special : it is used for line drawing (overline,
     * underline, strikethrough) and background.
     */
    if (character == static_cast<wchar_t>(-1)) {
        size_t width = _atlas.width();
        size_t height = _atlas.height();
        glm::ivec4 region = _atlas.allocateRegion(5, 5);
        std::array<unsigned char, 4*4*3> data;
        data.fill(std::numeric_limits<unsigned char>::max());
//        static unsigned char data[4*4*3] = {
//            
//            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
//            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
//            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
//            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
        if ( region.x < 0 )
        {
            return nullptr;
        }
        
        _atlas.setRegion(region.x, region.y, 4, 4, data.data(), 0);
        
        Glyph* glyph = new Glyph(static_cast<wchar_t>(-1));
        glyph->_topLeft = glm::vec2(
            (region.x+2)/static_cast<float>(width),
            (region.y+2)/static_cast<float>(height)
        );
        glyph->_bottomRight = glm::vec2(
            (region.x+3)/static_cast<float>(width),
            (region.y+3)/static_cast<float>(height)
        );

        _glyphs.push_back(glyph);

        return glyph; //*(texture_glyph_t **) vector_back( self->glyphs );
    }
    
    /* Glyph has not been already loaded */
    size_t nGlyphNotLoaded = loadGlyphs({character});
    if (nGlyphNotLoaded == 0)
        return _glyphs.back();
    else {
        LERROR(nGlyphNotLoaded << " glyphs could not be loaded");
        return nullptr;
    }
}
    
std::string Font::name() const {
    return _name;
}
    
float Font::fontSize() const {
    return _pointSize;
}
    
float Font::height() const {
    return _height;
}
    
opengl::TextureAtlas& Font::atlas() {
    return _atlas;
}
    
size_t Font::loadGlyphs(const std::vector<wchar_t>& glyphs) {
    size_t x, y, w, h;
    
    FT_Glyph ft_glyph;
    FT_GlyphSlot slot;
    FT_Bitmap ft_bitmap;
    
    FT_UInt glyph_index;
    
    size_t missed = 0;
    
    size_t width  = _atlas.width();
    size_t height = _atlas.height();
    size_t depth  = _atlas.depth();
    
    FT_Library library;
    FT_Face face;
    bool success = loadFace(_pointSize, library, face);
    if (!success)
        return glyphs.size();
    
    /* Load each glyph */
    for (size_t i = 0; i < glyphs.size(); ++i) {
        bool found = false;
        /* Check if charcode has been already loaded */
        for (size_t j = 0; j < _glyphs.size(); ++j ) {
            Glyph* glyph = _glyphs[j];
            // If charcode is -1, we don't care about outline type or thickness
            // if( (glyph->charcode == charcodes[i])) {
            if ((glyph->_charcode == glyphs[i]) &&
               ((glyphs[i] == (wchar_t)(-1) ) ||
                ((glyph->_outline == _outlineType) &&
                 (glyph->_outline_thickness == _outlineThickness))))
            {
                found = true;
                break;
            }
        }
        
        if (found)
            continue;
        
        FT_Int32 flags = 0;
        int ft_glyph_top = 0;
        int ft_glyph_left = 0;
        glyph_index = FT_Get_Char_Index(face, glyphs[i]);
        // WARNING: We use texture-atlas depth to guess if user wants
        //          LCD subpixel rendering
        
        if (_outlineType != Outline::None)
            flags |= FT_LOAD_NO_BITMAP;
        else
            flags |= FT_LOAD_RENDER;
        
        if (!_autoHinting)
            flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
        else
            flags |= FT_LOAD_FORCE_AUTOHINT;
        
        if (depth == 3) {
            FT_Library_SetLcdFilter(library, FT_LCD_FILTER_LIGHT);
            flags |= FT_LOAD_TARGET_LCD;
            if (_lcdFiltering)
                FT_Library_SetLcdFilterWeights(library, _lcdWeights.data());
        }
        
        FT_Error error = FT_Load_Glyph(face, glyph_index, flags);
        if (error) {
            LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            return glyphs.size() - i;
        }
        
        
        if (_outlineType == Outline::None) {
            slot            = face->glyph;
            ft_bitmap       = slot->bitmap;
            ft_glyph_top    = slot->bitmap_top;
            ft_glyph_left   = slot->bitmap_left;
        }
        else {
            FT_Stroker stroker;
            FT_BitmapGlyph ft_bitmap_glyph;
            FT_Error error = FT_Stroker_New(library, &stroker);
            if (error) {
                LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return 0;
            }
            
            FT_Stroker_Set(stroker,
                           static_cast<int>(_outlineThickness) * static_cast<int>(HighResolution),
                           FT_STROKER_LINECAP_ROUND,
                           FT_STROKER_LINEJOIN_ROUND,
                           0);
            error = FT_Get_Glyph(face->glyph, &ft_glyph);
            if (error) {
                LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return 0;
            }
            
            switch (_outlineType) {
                case Outline::None:
                    break;
                case Outline::Line:
                    error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
                    break;
                case Outline::Inner:
                    error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 0, 1);
                    break;
                case Outline::Outer:
                    error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 1, 1);
                    break;
            }
            if (error) {
                LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return 0;
            }
            
            if (depth == 1) {
                error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
                if (error) {
                    LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                    FT_Done_Face(face);
                    FT_Stroker_Done(stroker);
                    FT_Done_FreeType(library);
                    return 0;
                }
            }
            else {
                error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_LCD, 0, 1);
                if (error) {
                    LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                    FT_Done_Face(face);
                    FT_Stroker_Done(stroker);
                    FT_Done_FreeType(library);
                    return 0;
                }
            }
            ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
            ft_bitmap       = ft_bitmap_glyph->bitmap;
            ft_glyph_top    = ft_bitmap_glyph->top;
            ft_glyph_left   = ft_bitmap_glyph->left;
            FT_Stroker_Done(stroker);
        }
        
        
        // We want each glyph to be separated by at least one black pixel
        // (for example for shader used in demo-subpixel.c)
        w = ft_bitmap.width/depth + 1;
        h = ft_bitmap.rows + 1;
        glm::ivec4 region = _atlas.allocateRegion(w, h);
        if (region.x < 0) {
            missed++;
            LERROR("Texture atlas is full");
            continue;
        }
        w = w - 1;
        h = h - 1;
        x = region.x;
        y = region.y;
        _atlas.setRegion(x, y, w, h, ft_bitmap.buffer, ft_bitmap.pitch);
        
        Glyph* glyph = new Glyph(glyphs[i]);
        glyph->_width = w;
        glyph->_height = h;
        glyph->_outline = _outlineType;
        glyph->_outline_thickness = _outlineThickness;
        glyph->_offset_x = ft_glyph_left;
        glyph->_offset_y = ft_glyph_top;
        glyph->_topLeft = glm::vec2(
            x/static_cast<float>(width),
            y/static_cast<float>(height)
        );
        glyph->_bottomRight = glm::vec2(
            (x + glyph->_width)/static_cast<float>(width),
            (y + glyph->_height)/static_cast<float>(height)
                                        
        );
        
        // Discard hinting to get advance
        FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
        slot = face->glyph;
        glyph->_advance_x = slot->advance.x / HighResolution;
        glyph->_advance_y = slot->advance.y / HighResolution;
        
        _glyphs.push_back(glyph);
        
        if (_outlineType != Outline::None)
            FT_Done_Glyph(ft_glyph);
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
    
    bool success = loadFace(_pointSize, library, face);
    if (!success)
        return;
    
    /* For each glyph couple combination, check if kerning is necessary */
    /* Starts at index 1 since 0 is for the special backgroudn glyph */
    for (size_t i = 1; i < _glyphs.size(); ++i) {
        Glyph* glyph = _glyphs[i];
        FT_UInt glyphIndex = FT_Get_Char_Index(face, glyph->_charcode);
        glyph->_kerning.clear();
        
        for (size_t j = 1; j < _glyphs.size(); ++j) {
            Glyph* prevGlyph = _glyphs[j];
            FT_UInt prevIndex = FT_Get_Char_Index(face, prevGlyph->_charcode);
            FT_Vector kerning;
            FT_Get_Kerning(face, prevIndex, glyphIndex, FT_KERNING_UNFITTED, &kerning);
            if (kerning.x != 0) {
                glyph->_kerning[prevGlyph->_charcode] = kerning.x / (HighResolution*HighResolution);
            }
        }
    }
    
    FT_Done_Face( face );
    FT_Done_FreeType( library );
}
    
//bool Font::getFace(float size, FT_Library* library, FT_Face* face) {
//    return loadFace(size, library, face);
//}
//    
//bool Font::getFace(FT_Library* library, FT_Face* face) {
//    return getFace(_pointSize, library, face);
//}
//    
//bool Font::getHiResFace(FT_Library* library, FT_Face* face) {
//    return getFace(_pointSize * 100.f, library, face);
//}
//    
bool Font::loadFace(float size, FT_Library& library, FT_Face& face) {
}

    
} // namespace fontrendering
} // namespace ghoul
