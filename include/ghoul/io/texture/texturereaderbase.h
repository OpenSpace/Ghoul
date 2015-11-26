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

#ifndef __TEXTUREREADERBASE_H__
#define __TEXTUREREADERBASE_H__

#include <ghoul/misc/exception.h>

#include <memory>
#include <string>
#include <vector>

namespace ghoul {
namespace opengl {
class Texture;
}
    
namespace io {

/**
 * The base class for reading textures from hard disk into a Texture object. Each reader
 * must specify the extensions that is supports as well as provide a loadTexture overload
 * to handle the files.
 */
class TextureReaderBase {
public:
    /// The exception that gets thrown if there was an error loading the Texture
    struct TextureLoadException : public RuntimeError {
        explicit TextureLoadException(std::string name, std::string message,
            const TextureReaderBase* reader);
        
        /// The filename that caused the exception to be thrown
        std::string filename;
        
        /// The error message that occurred
        std::string message;
        
        /// The TextureReaderBase that caused the exception
        const TextureReaderBase* reader;
    };
    
    /// Default virtual destructor
    virtual ~TextureReaderBase();
    
    /**
     * Loads the texture \p filename from disk and returns the loaded Texture.
     * \param filename The texture that should be loaded from the hard disk
     * \return The loaded Texture object
     * \pre \p filename must not be empty
     * \pre The extension of \p filename must be among the supported extensions as
     * reported by supportedExtensions
     * \throw TextureLoadException If there was an error loading the texture
     */
    virtual std::unique_ptr<opengl::Texture> loadTexture(std::string filename) const = 0;
    
    /**
     * Returns a list of all extensions that this TextureReaderBase supports.
     * \return A list of all extensions that this TextureReaderBase supports
     */
	virtual std::vector<std::string> supportedExtensions() const = 0;
};

} // namespace io
} // namespace ghoul

#endif // __TEXTUREREADERBASE_H__
