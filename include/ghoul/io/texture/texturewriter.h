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

#ifndef __GHOUL___TEXTUREWRITER___H__
#define __GHOUL___TEXTUREWRITER___H__

#include <ghoul/misc/exception.h>
#include <memory>
#include <string>
#include <vector>

namespace ghoul::opengl { class Texture; }

namespace ghoul::io {

class TextureWriterBase;

/**
 * This class manages multiple TextureWriterBase and makes them available through one
 * method saveTexture. TextureWriterBases are added through the method addWriter. The
 * class provides a static member, but also allows users to create local variants.
 * TextureWriterBases can be reused between multiple TextureWriters
 */
class TextureWriter {
public:
    /// Exception that gets thrown when there is no writer for the provided \p extension
    struct MissingWriterException : public RuntimeError {
        explicit MissingWriterException(std::string extension);
        std::string fileExtension;
    };

    /**
     * Returns the static variant of the TextureWriter.
     *
     * \return The static variant of the TextureWriter
     */
    static TextureWriter& ref();

    /**
     * Saves the provided \p texture into the \p filename on disk. The correct
     * TextureWriterBase is determined by the extension of the \p filename.
     *
     * \param texture The Texture that is to be written to disk
     * \param filename The target filename for \p filename. The extension of the
     *        \p filename determines the TextureWriterBase
     *
     * \throw TextureWriteException If there was an error writing the \p filename
     * \throw MissingWriterException If there was no writer for the specified \p filename
     * \pre \p filename must not be empty
     * \pre \p filename must have an extension
     * \pre At least one TextureWriterBase must have been added to the TextureWriter
     *      before (addWriter)
     */
    void saveTexture(const opengl::Texture& texture, const std::string& filename);

    /**
     * Adds the \p writer to this TextureWriter and makes it available through subsequent
     * calls to saveTexture. If an extension is supported by multiple TextureWriterBases,
     * the TextureWriterBase that was added first will be used.
     *
     * \param writer The writer that is to be added to this TextureWriter
     *
     * \pre \p writer must not have been added to this TextureWriter before
     */
    void addWriter(std::unique_ptr<TextureWriterBase> writer);

private:
    /**
     * Returns the TextureWriterBase that is responsible for the provided extension.
     *
     * \param extension The extension for which the TextureWriterBase should be returned
     * \return The first TextureWriterBase that can write the provided \p extension, or
     *         \c nullptr if no such file exists
     */
    TextureWriterBase* writerForExtension(const std::string& extension);

    /// The list of all registered writers
    std::vector<std::unique_ptr<TextureWriterBase>> _writers;
};

} // namespace ghoul::io

#endif // __GHOUL___TEXTUREWRITER___H__
