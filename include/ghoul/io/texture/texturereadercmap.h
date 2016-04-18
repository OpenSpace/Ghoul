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

#ifndef __TEXTUREREADERCMAP_H__
#define __TEXTUREREADERCMAP_H__

#include <ghoul/io/texture/texturereaderbase.h>

namespace ghoul {
namespace io {

/**
 * Reads a one dimensional texture from a cmap (color map) file. The format of the cmap
 * files have the width of the Texture in the first non-comment, non-empty line and is
 * followed by <code>width</code> number of lines that contain four values for red, green,
 * blue, and alpha each. If the file is malformed, an TextureLoadException is thrown
 */
class TextureReaderCMAP : public TextureReaderBase {
public:
    /**
     * Loads the passed CMAP file.
     * \param filename The CMAP file that is loaded
     * \return The loaded Texture
     * \throws TextureLoadException If the file is malformed
     * \throws std::ifstream::failure If the file could not be opened
     * \pre \p filename must not be empty
     */
    std::unique_ptr<opengl::Texture> loadTexture(std::string filename) const override;
    std::unique_ptr<opengl::Texture> loadTextureFromMemory(const std::string& buffer) const override;
    
    /**
     * Returns the supported extension (<code>cmap</code>)
     * \return The supported extension
     */
    std::vector<std::string> supportedExtensions() const override;
};

} // namespace io
} // namespace ghoul

#endif // __TEXTUREREADERCMAP_H__
