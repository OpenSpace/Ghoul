/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#ifndef __GHOUL___TEXTUREREADERDEVIL___H__
#define __GHOUL___TEXTUREREADERDEVIL___H__

#include <ghoul/io/texture/texturereaderbase.h>

#include <memory>

namespace ghoul {
namespace io {

#ifdef GHOUL_USE_DEVIL

/**
 * Loads the texture using the DevIL library. For a list of supported image formats, see 
 * http://openil.sourceforge.net/features.php
 */
class TextureReaderDevIL : public TextureReaderBase {
public:
    /**
    * Loads the texture \p filename using the DevIL library from disk and returns the
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
    * Loads a Texture from the memory pointed at by \p memory using the DevIL library. The
    * memory block must contain at least \p size number of bytes.
    * \param memory The memory that contains the bytes of the Texture to be loaded
    * \param size The number of bytes contained in \p memory
    * \throw TextureLoadException If there was an error reading the \p memory
    * \pre \p memory must not be <code>nullptr</code>
    * \pre \p size must be > 0
    */
    std::unique_ptr<opengl::Texture> loadTexture(void* memory,
        size_t size) const override;

    /**
     * Returns the supported file extensions
     * \return The supported file extensions
     * \sa http://openil.sourceforge.net/features.php
     */
    std::vector<std::string> supportedExtensions() const override;

private:
    std::unique_ptr<opengl::Texture> loadTextureInternal(const std::string& source) const;
};

#endif // GHOUL_USE_DEVIL

} // namespace io
} // namespace ghoul

#endif // __GHOUL___TEXTUREREADERDEVIL___H__
