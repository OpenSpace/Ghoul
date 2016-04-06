/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2016                                                               *
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

#ifndef __TEXTUREREADER_H__
#define __TEXTUREREADER_H__

#include <memory>
#include <string>
#include <vector>

#include <ghoul/misc/exception.h>
#include <ghoul/opengl/texture.h>

namespace ghoul {
namespace io {

class TextureReaderBase;

/**
 * This class manages multiple TextureReaderBase and makes them available through one
 * method loadTexture. TextureReaderBases are added through the method addReader. The
 * class provides a static member, but also allows users to create local variants.
 * TextureReaderBases can be reused between multiple TextureReaders
 */
class TextureReader {
public:
    /// Exception that gets thrown when there is no reader for the provided \p extension
    struct MissingReaderException : public RuntimeError {
        explicit MissingReaderException(std::string extension);
        std::string fileExtension;
    };
    
    /**
     * Returns the static variant of the TextureReader
     * \return The static variant of the TextureReader
     */
    static TextureReader& ref();
    
    /**
     * Loads the provided \p filename into a Texture and returns it. The correct
     * TextureReaderBase is determined by the extension of the \p filename.
     * \param filename The name of the file which should be loaded into a textrure
     * \throw TextureLoadException If there was an error reading the \p filename
     * \throw MissingReaderException If there was no reader for the specified \p filename
     * \pre \p filename must not be empty
     * \pre \p filename must have an extension
     * \pre At least one TextureReaderBase must have been added to the TextureReader
     * before (addReader)
     */
    std::unique_ptr<opengl::Texture> loadTexture(const std::string& filename);
    std::unique_ptr<opengl::Texture> loadTextureFromMemory(const std::string& buffer);
    
    /**
     * Adds the \p reader to this TextureReader and makes it available through subsequent
     * calls to loadTexture. If an extension is supported by multiple TextureReaderBases,
     * the TextureReaderBase that was added first will be used.
     * \param reader The reader that is to be added to this TextureReader
     * \pre \p reader must not have been added to this TextureReader before
     */
    void addReader(std::shared_ptr<TextureReaderBase> reader);
    
    /**
     * Returns a list of all the previously registered TextureReaderBases.
     * \return A list of all the previously registered TextureReaderBases
     */
    std::vector<std::shared_ptr<TextureReaderBase>> readers() const;

private:
    /**
     * Returns the TextureReaderBase that is responsible for the provided extension.
     * \param extension The extension for which the TextureReaderBase should be returned
     * \throw MissingReaderException If there was no reader for the specified \p extension
     */
    TextureReaderBase* readerForExtension(const std::string& extension);
    
    /// The list of all registered readers
    std::vector<std::shared_ptr<TextureReaderBase>> _readers;
};

} // namespace io
} // namespace ghoul

#endif // __TEXTUREREADER_H__
