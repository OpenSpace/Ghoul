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

#ifndef __GHOUL___TEXTUREREADERCMAP___H__
#define __GHOUL___TEXTUREREADERCMAP___H__

#include <ghoul/io/texture/texturereaderbase.h>

namespace ghoul::io {

/**
 * Reads a one dimensional texture from a cmap (color map) file. The format of the cmap
 * files have the width of the Texture in the first non-comment, non-empty line and is
 * followed by <code>width</code> number of lines that contain four values for red, green,
 * blue, and alpha each. If the file is malformed, an TextureLoadException is thrown
 */
class TextureReaderCMAP : public TextureReaderBase {
public:
    /**
    * Loads the texture \p filename in CMAP format from disk and returns the loaded
    * Texture.
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
    * Loads a Texture from the memory pointed at by \p memory in CMAP format. The memory
    * block must contain at least \p size number of bytes.
    * \param memory The memory that contains the bytes of the Texture to be loaded
    * \param size The number of bytes contained in \p memory
    * \throw TextureLoadException If there was an error reading the \p memory
    * \pre \p memory must not be <code>nullptr</code>
    * \pre \p size must be > 0
    */
    std::unique_ptr<opengl::Texture> loadTexture(void* memory,
                                                 size_t size) const override;

    /*
     * Returns the supported extension (<code>cmap</code>)
     * \return The supported extension
     */
    std::vector<std::string> supportedExtensions() const override;
};

} // namespace ghoul::io

#endif // __GHOUL___TEXTUREREADERCMAP___H__
