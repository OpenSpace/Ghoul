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

#ifndef __GHOUL___TEXTUREREADERBASE___H__
#define __GHOUL___TEXTUREREADERBASE___H__

#include <ghoul/misc/exception.h>
#include <memory>
#include <string>
#include <vector>

namespace ghoul::opengl { class Texture; }

namespace ghoul::io {

/**
 * The base class for reading textures from hard disk into a Texture object. Each reader
 * must specify the extensions that is supports as well as provide a loadTexture overload
 * to handle the files.
 */
class TextureReaderBase {
public:
    /// The exception that gets thrown if there was an error loading the Texture
    struct TextureLoadException : public RuntimeError {
        explicit TextureLoadException(std::string name, std::string msg,
            const TextureReaderBase* r);

        /// The filename that caused the exception to be thrown
        std::string filename;

        /// The error message that occurred
        std::string message;

        /// The TextureReaderBase that caused the exception
        const TextureReaderBase* reader;
    };

    /// Default virtual destructor
    virtual ~TextureReaderBase() = default;

    /**
     * Loads the texture \p filename from disk and returns the loaded Texture.
     *
     * \param filename The texture that should be loaded from the hard disk
     * \return The loaded Texture object
     *
     * \throw TextureLoadException If there was an error loading the texture
     * \pre \p filename must not be empty
     * \pre The extension of \p filename must be among the supported extensions as
     *      reported by supportedExtensions
     */
    virtual std::unique_ptr<opengl::Texture> loadTexture(
        const std::string& filename) const = 0;

    /**
    * Loads a Texture from the memory pointed at by \p memory. The memory block must
    * contain at least \p size number of bytes.
     *
    * \param memory The memory that contains the bytes of the Texture to be loaded
    * \param size The number of bytes contained in \p memory
    *
    * \throw TextureLoadException If there was an error reading the \p memory
    * \pre \p memory must not be <code>nullptr</code>
    * \pre \p size must be > 0
    */
    virtual std::unique_ptr<opengl::Texture> loadTexture(void* memory,
        size_t size) const = 0;

    /**
     * Returns a list of all extensions that this TextureReaderBase supports.
     *
     * \return A list of all extensions that this TextureReaderBase supports
     */
    virtual std::vector<std::string> supportedExtensions() const = 0;
};

} // namespace ghoul::io

#endif // __GHOUL___TEXTUREREADERBASE___H__
