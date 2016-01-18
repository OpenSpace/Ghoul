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

#include <ghoul/io/texture/texturewritersoil.h>

#ifdef GHOUL_USE_SOIL

#include <ghoul/filesystem/file.h>
#include <ghoul/opengl/texture.h>

#include <algorithm>
#include <SOIL.h>

namespace ghoul {
namespace io {

void TextureWriterSOIL::saveTexture(const opengl::Texture& texture,
                                                        const std::string& filename) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");
    
    auto extensions = supportedExtensions();
    std::string extension = ghoul::filesystem::File(filename).fileExtension();
    ghoul_assert(
        std::find(extensions.begin(), extensions.end(), extension) != extensions.end(),
        "Extension of the filename must be supported"
    );
    
    int width = texture.width();
    int height = texture.height();
    
    int type;
    
    if (extension == "bmp") {
        type = SOIL_SAVE_TYPE_BMP;
    } else if (extension == "tga") {
        type = SOIL_SAVE_TYPE_TGA;
    } else if (extension == "dds") {
        type = SOIL_SAVE_TYPE_DDS;
    }
    
    const unsigned char* data = static_cast<const unsigned char*>(texture.pixelData());
    int result = SOIL_save_image(
        filename.c_str(),
        type,
        width,
        height,
        texture->numberOfChannels(),
        data
    );
    if (result == 0)
        throw TextureWriteException(filename, "Error writing Texture", this);
}
    
std::vector<std::string> TextureWriterSOIL::supportedExtensions() const {
    // taken from http://www.lonesock.net/soil.html
    return {
        "bmp",
		"tga",
        "dds"
	};
}
    
} // namespace opengl
} // namespace ghoul

#endif // GHOUL_USE_SOIL
