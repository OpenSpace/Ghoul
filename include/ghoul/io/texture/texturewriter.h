/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2026                                                               *
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

#ifndef __GHOUL___TEXTUREWRITER___H__
#define __GHOUL___TEXTUREWRITER___H__

#include <ghoul/misc/exception.h>
#include <string>
#include <vector>

namespace ghoul::opengl { class Texture; }

namespace ghoul::io {

/**
 * A utility class for writing OpenGL textures to image files on disk using the STB
 * image writer library. This class provides a centralized interface for saving textures
 * in various common image formats (JPEG, PNG, BMP, TGA).
 *
 * When writing, the appropriate image format is automatically determined from the file
 * extension provided in the filename.
 *
 * Supported file formats include:
 * - JPEG (.jpeg, .jpg)
 * - PNG (.png)
 * - BMP (.bmp)
 * - TGA (.tga)
 *
 * \note 3D textures are not supported and will result in an error.
 * \note The class automatically creates the target directory if it doesn't exist.
 */
class TextureWriter {
public:
    /**
     * Exception that gets thrown when the provided \p extension is not supported.
     */
    struct MissingWriterException final : public RuntimeError {
        explicit MissingWriterException(std::string extension);
        const std::string fileExtension;
    };

    /**
     * The exception that gets thrown if there was an error writing the Texture.
     */
    struct TextureWriteException final : public RuntimeError {
        explicit TextureWriteException(std::string name, std::string msg);

        /// The filename that caused the exception to be thrown
        const std::string filename;

        /// The error message that occurred
        const std::string errorMessage;
    };

    /**
     * Saves the provided \p texture into the \p filename on disk. The image file format
     * is determined by the extension of the \p filename.
     *
     * \param texture The Texture that is to be written to disk
     * \param filename The target filename for \p filename.
     *
     * \throw TextureWriteException If there was an error writing the \p filename
     * \throw MissingWriterException If the extension in the \p filename is not supported
     * \pre \p filename must not be empty
     * \pre \p filename must have an extension
     */
    static void saveTexture(const opengl::Texture& texture, const std::string& filename);

    /**
     * Returns Whether the provided file \p extension is supported by this TextureWriter.
     *
     * \return True if the provided \p extension is supported, false otherwise
     */
    static bool isSupportedExtension(const std::string& extension);

    /**
     * Returns the supported file extensions.
     *
     * \return The supported file extensions
     */
    static std::vector<std::string> supportedExtensions();
};

} // namespace ghoul::io

#endif // __GHOUL___TEXTUREWRITER___H__
