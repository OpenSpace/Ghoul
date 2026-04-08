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

#ifndef __GHOUL___TEXTUREREADER___H__
#define __GHOUL___TEXTUREREADER___H__

#include <ghoul/glm.h>
#include <ghoul/misc/exception.h>
#include <ghoul/opengl/texture.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace ghoul::io {

/**
 * A utility class for loading texture data from files or memory into OpenGL Texture
 * objects using the STB image library. This class provides a centralized interface for
 * loading textures from various common image formats and converting them into GPU-ready
 * texture objects.
 *
 * The class uses the singleton pattern through its static `ref()` method, providing a
 * globally accessible instance.
 *
 * The class supports loading textures with 1, 2, or 3 dimensions and provides
 * functionality to:
 * - Load textures directly from file paths
 * - Load textures from raw memory buffers
 * - Query image dimensions without full texture creation
 * - Configure texture sampler settings (filtering, wrapping, etc.)
 *
 * Supported file formats include:
 * - JPEG (.jpeg, .jpg)
 * - PNG (.png)
 * - BMP (.bmp)
 * - TGA (.tga)
 * - PSD (.psd)
 * - GIF (.gif)
 * - HDR (.hdr)
 * - PIC (.pic)
 * - PNM (.ppm, .pgm)
 */
class TextureReader {
public:
    /**
     * Exception that gets thrown when the provided \p extension is not supported.
     */
    struct MissingReaderException final : public RuntimeError {
        explicit MissingReaderException(std::string extension,
            std::filesystem::path file_);

        const std::string fileExtension;
        const std::filesystem::path file;
    };

    /**
     * The exception that gets thrown if there was an error loading the Texture.
     */
    struct TextureLoadException final : public RuntimeError {
        explicit TextureLoadException(std::filesystem::path name, std::string msg);

        /// The filename that caused the exception to be thrown
        const std::filesystem::path filename;

        /// The error message that occurred
        const std::string errorMessage;
    };

    /**
     * Exception that gets thrown when an invalid load result occurs.
     */
    struct InvalidLoadException final : public RuntimeError {
        explicit InvalidLoadException(void* memory, size_t size);

        const void* _memory;
        const size_t _size;
    };

    /**
     * Returns the static variant of the TextureReader.
     *
     * \return The static variant of the TextureReader
     */
    static TextureReader& ref();

    /**
     * Loads the provided \p filename into a Texture and returns it. The image format is
     * determined by the extension of the \p filename.
     *
     * \param filename The name of the file which should be loaded into a texture
     * \param nDimensions The number of dimensions of the texture that are returned when
     *        using this function. This parameter is necessary as it is not always
     *        possible to automatically detect this based on the image information. For
     *        example, someone might want to load a 128x1 texture but use it as a 2D
     *        texture instead
     * \param samplerSettings The settings that should be used for the Texture that is
     *        created from the contents of the \p filename
     *
     * \throw TextureLoadException If there was an error reading the \p filename
     * \throw MissingReaderException If the extension in the \p filename is not supported
     * \pre \p filename must not be empty
     * \pre \p filename must have an extension
     * \pre \p nDimensions The number of texture dimension must be 1, 2, or 3
     */
    std::unique_ptr<opengl::Texture> loadTexture(const std::filesystem::path& filename,
        int nDimensions, opengl::Texture::SamplerInit samplerSettings = {});

    /**
     * Loads a Texture from the memory pointed at by \p memory. The memory block must
     * contain at least \p size number of bytes.
     *
     * \param memory The memory that contains the bytes of the Texture to be loaded
     * \param size The number of bytes contained in \p memory
     * \param nDimensions The number of dimensions of the texture that are returned when
     *        using this function. This parameter is necessary as it is not always
     *        possible to automatically detect this based on the image information. For
     *        example, someone might want to load a 128x1 texture but use it as a 2D
     *        texture instead
     * \param format The format of the image pointed to by \p memory. This parameter
     *        should be the same as the usual file extension for the image and is
     *        used to determine if the file type is supported by the reader
     * \param samplerSettings The settings that should be used for the Texture that is
     *        created from the contents of the \p memory
     *
     * \throw TextureLoadException If there was an error reading the \p memory
     * \throw MissingReaderException If the extension in the \p filename is not supported
     * \pre \p memory must not be `nullptr`
     * \pre \p size must be > 0
     * \pre \p nDimensions The number of texture dimension must be 1, 2, or 3
     */
    std::unique_ptr<opengl::Texture> loadTexture(void* memory, size_t size,
        int nDimensions, opengl::Texture::SamplerInit samplerSettings = {},
        const std::string& format = "");

    /**
     * Returns the size of the image pointed to by \p filename.
     *
     * \param filename The image file that should be inspected
     * \return The size of the image in pixels
     *
     * \throw TextureLoadException If there was an error loading the texture
     * \pre \p filename must not be empty
     * \pre The extension of \p filename must be among the supported extensions as
     *      reported by `supportedExtensions`
     */
    glm::ivec2 imageSize(const std::filesystem::path& filename) const;

    /**
     * Returns Whether the provided file \p extension is supported by this reader.
     *
     * \return True if the provided \p extension is supported, false otherwise
     */
    bool isSupportedExtension(const std::string& extension) const;

    /**
     * Returns a list of all the extensions that are supported. If a file with an
     * extension included in this list is passed to the loadTexture file and the file
     * is not corrupted, it will be successfully loaded.
     *
     * \return A list of all supported extensions
     */
    std::vector<std::string> supportedExtensions() const;
};

} // namespace ghoul::io

#endif // __GHOUL___TEXTUREREADER___H__
