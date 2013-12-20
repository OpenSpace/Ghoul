/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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

#include <string>

namespace ghoul {
namespace opengl {

class Texture;

/**
 * Loads the texture located at <code>filename</code>, using the DevIL library, and 
 * creates a \see Texture object from it. For a list of supported image formats, see <url>
 * http://openil.sourceforge.net/features.php</url>. If an image can not be loaded, a null
 * pointer is returned and an error is logged. Otherwise a \see Texture of appropriate 
 * format is created and returned. The ownership of the texture passes to the caller. The 
 * Texture will not automatically be uploaded to the graphics card.
 * \param filename The file name of the image that should be loaded
 * \param useCompression <code>true</code> if the returned Texture should be in a 
 * compressed format. <code>false</code> will use a non-compressed format
 * \return The created Texture or a null pointer if the loading failed
 */
Texture* loadTexture(const std::string& filename, bool useCompression = false);

} // namespace opengl
} // namespace ghoul

#endif
