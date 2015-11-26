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

#ifndef __TEXTUREREADERFREEIMAGE_H__
#define __TEXTUREREADERFREEIMAGE_H__

#include <ghoul/io/texture/texturereaderbase.h>

#include <memory>
#include <string>

namespace ghoul {
namespace io {

#ifdef GHOUL_USE_FREEIMAGE

/**
 * Loads the texture from the provided file using the FreeImage library and creates a
 * Texture object from it.
 * \sa http://freeimage.sourceforge.net
 */
class TextureReaderFreeImage : public TextureReaderBase {
public:
    /**
     * Loads the \p filename and creates a Texture from it.
     * \param filename The texture file to be loaded
     * \return The loaded Texture object
     * \pre \p filename must not be empty
     * \throw TextureLoadException If there was an error loading the Texture file
     */
    std::unique_ptr<opengl::Texture> loadTexture(std::string filename) const override;
    
    /**
     * Returns the supported extensions (<code>bmp</code>, <code>cut</code>,
     * <code>dds</code>, <code>exr</code>, <code>g3</code>, <code>gif</code>,
     * <code>hdr</code>, <code>ico</code>, <code>iff</code>, <code>jbig</code>,
     * <code>jng</code>, <code>jpg</code>, <code>jpeg</code>, <code>koala</code>,
     * <code>mng</code>, <code>pcx</code>, <code>pbm</code>, <code>pgm</code>,
     * <code>ppm</code>, <code>pfm</code>, <code>png</code>, <code>pict</code>,
     * <code>psd</code>, <code>raw</code>, <code>ras</code>, <code>sgi</code>,
     * <code>targa</code>, <code>tif</code>, <code>tiff</code>, <code>wbmp</code>,
     * <code>webp</code>, <code>xbm</code>, <code>xpm</code>)
     * \sa http://freeimage.sourceforge.net
     * \return The supported extension
     */
	std::vector<std::string> supportedExtensions() const override;
};

#endif // GHOUL_USE_FREEIMAGE

} // namespace io
} // namespace ghoul

#endif // __TEXTUREREADERFREEIMAGE_H__
