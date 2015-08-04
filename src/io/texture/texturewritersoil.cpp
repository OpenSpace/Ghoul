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
#include <ghoul/logging/logmanager.h>
#include <ghoul/glm.h>
#include <vector>

#include <SOIL.h>

namespace {
    const std::string _loggerCat = "TextureWriterSOIL";
}

namespace ghoul {
namespace io {
namespace impl {

void TextureWriterSOIL::saveTexture(const opengl::Texture* texture, const std::string& filename) const {
    using opengl::Texture;
    
    int width = texture->width(),
        height = texture->height();
    
    std::string extension = ghoul::filesystem::File(filename).fileExtension();
    int type;
    
    if (extension == "bmp") {
        type = SOIL_SAVE_TYPE_BMP;
    } else if (extension == "tga") {
        type = SOIL_SAVE_TYPE_TGA;
    } else if (extension == "dds") {
        type = SOIL_SAVE_TYPE_DDS;
    } else {
        LERROR("Extension \"" << extension << "\" not supported.");
        return;
    }

    const unsigned char* data = static_cast<const unsigned char*>(texture->pixelData());
    SOIL_save_image(filename.c_str(), type, width, height, texture->numberOfChannels(), data);
}
    
    std::set<std::string> TextureWriterSOIL::supportedExtensions() const {
        // taken from http://www.lonesock.net/soil.html
        return {
                "bmp",  // load & save
		"tga",  // load & save
                "dds",  // load & save
	};
    }
    
} // namespace impl
} // namespace opengl
} // namespace ghoul

#endif // GHOUL_USE_SOIL
