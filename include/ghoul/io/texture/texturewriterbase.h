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

#ifndef __GHOUL___TEXTUREWRITERBASE___H__
#define __GHOUL___TEXTUREWRITERBASE___H__

#include <ghoul/misc/exception.h>
#include <ghoul/opengl/texture.h>
#include <string>
#include <vector>

namespace ghoul::io {

/**
 * This is the base class for all classes that cna be used to write textures to disk. Each
 * reader needs to list all supported extensions (#supportedExtensions) and overwrite the
 * saveTexture method.
 */
class TextureWriterBase {
public:
    /// The exception that gets thrown if there was an error writing the Texture
    struct TextureWriteException : public RuntimeError {
        explicit TextureWriteException(std::string name, std::string msg,
            const TextureWriterBase& w);

        /// The filename that caused the exception to be thrown
        std::string filename;

        /// The error message that occurred
        std::string message;

        /// The TextureWriterBase that caused the exception
        const TextureWriterBase& writer;
    };

    /// Default virtual destructor
    virtual ~TextureWriterBase() {}

    /**
     * Saves the \p texture to disk.
     *
     * \param texture The Texture to write to disk
     * \param filename The target file for the Texture
     *
     * \pre \p filename must not be empty
     * \pre The extension of \p filename must be among the supported extensions as
     *      reported by supportedExtensions
     * \throw TextureWriteException If there was an error writing the \p texture
     */
    virtual void saveTexture(const opengl::Texture& texture,
        const std::string& filename) const = 0;

    /**
     * Returns a list of all extensions that this TextureWriteException supports.
     *
     * \return A list of all extensions that this TextureWriteException supports
     */
    virtual std::vector<std::string> supportedExtensions() const = 0;
};

} // namespace ghoul::io

#endif // __GHOUL___TEXTUREWRITERBASE___H__
