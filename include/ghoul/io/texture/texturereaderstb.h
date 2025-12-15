/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2025                                                               *
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

#ifndef __GHOUL___TEXTUREREADERSTB___H__
#define __GHOUL___TEXTUREREADERSTB___H__

#include <ghoul/io/texture/texturereaderbase.h>

namespace ghoul::io {

/// Loads the texture using the Sean Barrett's stb_image library.
class TextureReaderSTB : public TextureReaderBase {
public:
    /**
     * Loads the texture \p filename using the STB library from disk and returns the
     * loaded Texture.
     *
     * \param filename The texture that should be loaded from the hard disk
     * \param nDimensions The number of dimensions of the texture that is created
     * \return The loaded Texture object
     *
     * \throw TextureLoadException If there was an error loading the texture
     * \pre \p filename must not be empty
     * \pre The extension of \p filename must be among the supported extensions as
     *      reported by supportedExtensions
     * \pre \p nDimensions must be 1, 2, or 3
     */
    std::unique_ptr<opengl::Texture> loadTexture(
        const std::filesystem::path& filename, int nDimensions) const override;

    /**
     * Loads a Texture from the memory pointed at by \p memory using the STB library. The
     * memory block must contain at least \p size number of bytes.
     *
     * \param memory The memory that contains the bytes of the Texture to be loaded
     * \param size The number of bytes contained in \p memory
     * \param nDimensions The number of dimensions of the texture that is created
     *
     * \throw TextureLoadException If there was an error reading the \p memory
     * \pre \p memory must not be `nullptr`
     * \pre \p size must be > 0
     * \pre \p nDimensions must be 1, 2, or 3
     */
    std::unique_ptr<opengl::Texture> loadTexture(void* memory,
        size_t size, int nDimensions) const override;

    /**
     * Returns the size of the image pointed to by \p filename.
     *
     * \param filename The image file that should be inspected
     * \return The size of the image in pixels
     *
     * \throw TextureLoadException If there was an error loading the texture
     * \pre \p filename must not be empty
     * \pre The extension of \p filename must be among the supported extensions as
     *      reported by supportedExtensions
     */
    virtual glm::ivec2 imageSize(const std::filesystem::path& filename) const;

    /**
     * Returns the supported file extensions.
     *
     * \return The supported file extensions
     */
    std::vector<std::string> supportedExtensions() const override;
};

} // namespace ghoul::io

#endif // __GHOUL___TEXTUREREADERSTB___H__
