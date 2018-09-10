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

#ifndef __GHOUL___TEXTUREREADER___H__
#define __GHOUL___TEXTUREREADER___H__

#include <ghoul/misc/exception.h>
#include <memory>
#include <string>
#include <vector>

namespace ghoul::opengl { class Texture; }

namespace ghoul::io {

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
        explicit MissingReaderException(std::string extension, std::string file);

        std::string fileExtension;
        std::string file;
    };

    /**
     * Returns the static variant of the TextureReader.
     *
     * \return The static variant of the TextureReader
     */
    static TextureReader& ref();

    /**
     * Loads the provided \p filename into a Texture and returns it. The correct
     * TextureReaderBase is determined by the extension of the \p filename.
     *
     * \param filename The name of the file which should be loaded into a texture
     *
     * \throw TextureLoadException If there was an error reading the \p filename
     * \throw MissingReaderException If there was no reader for the specified \p filename
     * \pre \p filename must not be empty
     * \pre \p filename must have an extension
     * \pre At least one TextureReaderBase must have been added to the TextureReader
     *      before (addReader)
     */
    std::unique_ptr<opengl::Texture> loadTexture(const std::string& filename);

    /**
     * Loads a Texture from the memory pointed at by \p memory. The memory block must
     * contain at least \p size number of bytes. The bytes are then passed to the actual
     * implementation of the texture reader to create the Textjure object. The optional
     * \p format parameter is used to disambiguate the cases where multiple TextureReaders
     * are registered. In this case, the \p format is used in the same way as the file
     * extension for the #loadTexture method.
     *
     * \param memory The memory that contains the bytes of the Texture to be loaded
     * \param size The number of bytes contained in \p memory
     * \param format The format of the image pointed to by \p memory. This parameter
     *        should be the same as the usual file extension for the image. However, this
     *        parameter is only used to determine which TextureReader is used for this
     *        memory, if multiple readers are registered.
     *
     * \throw TextureLoadException If there was an error reading the \p memory
     * \throw MissingReaderException If there was no reader for the specified \p filename
     * \pre \p memory must not be <code>nullptr</code>
     * \pre \p size must be > 0
     */
    std::unique_ptr<opengl::Texture> loadTexture(void* memory, size_t size,
        const std::string& format = "");

    /**
     * Returns a list of all the extensions that are supported by registered readers. If
     * a file with an extension included in this list is passed to the loadTexture file
     * and the file is not corrupted, it will be successfully loaded.
     *
     * \return A list of all supported extensions
     */
    std::vector<std::string> supportedExtensions();

    /**
     * Adds the \p reader to this TextureReader and makes it available through subsequent
     * calls to loadTexture. If an extension is supported by multiple TextureReaderBases,
     * the TextureReaderBase that was added first will be used.
     *
     * \param reader The reader that is to be added to this TextureReader
     *
     * \pre \p reader must not have been added to this TextureReader before
     */
    void addReader(std::unique_ptr<TextureReaderBase> reader);

private:
    /**
     * Returns the TextureReaderBase that is responsible for the provided extension.
     *
     * \param extension The extension for which the TextureReaderBase should be returned
     * \return The first match of a reader that can read the provided \p extension, or
     *         \c nullptr if no such reader exists
     */
    TextureReaderBase* readerForExtension(const std::string& extension);

    /// The list of all registered readers
    std::vector<std::unique_ptr<TextureReaderBase>> _readers;
};

} // namespace ghoul::io

#endif // __GHOUL___TEXTUREREADER___H__
