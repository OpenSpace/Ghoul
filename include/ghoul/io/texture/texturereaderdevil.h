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

#ifndef __TEXTUREREADERDEVIL_H__
#define __TEXTUREREADERDEVIL_H__

#include <ghoul/io/texture/texturereaderbase.h>

#include <memory>

namespace ghoul {
namespace io {

#ifdef GHOUL_USE_DEVIL

/**
 * Loads the texture using the DevIL library. For a list of supported image formats, see 
 * http://openil.sourceforge.net/features.php
 */
class TextureReaderDevIL : public TextureReaderBase {
public:
	/**
	 * Loads the passed \p filename by using the DevIL library.
	 * \param filename The file to load
	 * \return The loaded Texture
	 * \pre \p filename must not be empty
	 * \throws TextureLoadException If there was an error loading the Texture file
	 */
	std::unique_ptr<opengl::Texture> loadTexture(std::string filename) const override;
    std::unique_ptr<opengl::Texture> loadTextureFromMemory(const std::string& buffer) const override;

	/**
	 * Returns the supported file extensions
	 * \return The supported file extensions
	 * \sa http://openil.sourceforge.net/features.php
	 */
	std::vector<std::string> supportedExtensions() const override;
};

#endif // GHOUL_USE_DEVIL

} // namespace io
} // namespace ghoul

#endif // __TEXTUREREADERDEVIL_H__
