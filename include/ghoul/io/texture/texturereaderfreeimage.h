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

#ifndef __GHOUL___TEXTUREREADERFREEIMAGE___H__
#define __GHOUL___TEXTUREREADERFREEIMAGE___H__

#include <ghoul/io/texture/texturereaderbase.h>

#include <memory>
#include <string>

struct FIBITMAP;

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
    * Loads the texture \p filename using the FreeImage library from disk and returns the
    * loaded Texture.
    * \param filename The texture that should be loaded from the hard disk
    * \return The loaded Texture object
    * \throw TextureLoadException If there was an error loading the texture
    * \pre \p filename must not be empty
    * \pre The extension of \p filename must be among the supported extensions as
    * reported by supportedExtensions
    */
    std::unique_ptr<opengl::Texture> loadTexture(
        const std::string& filename) const override;


    /**
    * Loads a Texture from the memory pointed at by \p memory using the FreeImage library.
    * The memory block must contain at least \p size number of bytes.
    * \param memory The memory that contains the bytes of the Texture to be loaded
    * \param size The number of bytes contained in \p memory
    * \throw TextureLoadException If there was an error reading the \p memory
    * \pre \p memory must not be <code>nullptr</code>
    * \pre \p size must be > 0
    */
    std::unique_ptr<opengl::Texture> loadTexture(void* memory,
        size_t size) const override;

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

private:
    std::unique_ptr<opengl::Texture> loadTextureInternal(const std::string& source,
        FIBITMAP* dib) const;
};

#endif // GHOUL_USE_FREEIMAGE

} // namespace io
} // namespace ghoul

#endif // __GHOUL___TEXTUREREADERFREEIMAGE___H__
