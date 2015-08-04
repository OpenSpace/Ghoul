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

#ifndef __TEXTUREWRITERDSOIL_H__
#define __TEXTUREWRITERDSOIL_H__

#include <ghoul/io/texture/texturewriterbase.h>

namespace ghoul {
namespace io {
namespace impl {

#ifdef GHOUL_USE_SOIL

/**
 * Saves the texture using the Soil library. For a list of supported image formats, see 
 * http://www.lonesock.net/soil.html 
 */
class TextureWriterSOIL : public TextureWriterBase {
 public:
    void saveTexture(const opengl::Texture* texture, const std::string& filename) const override;
    std::set<std::string> supportedExtensions() const override;
};

#endif // GHOUL_USE_SOIL

} // namespace impl
} // namespace io
} // namespace ghoul

#endif // __TEXTUREWRITERSOIL_H__
