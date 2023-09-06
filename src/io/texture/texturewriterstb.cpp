/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2023                                                               *
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

#include <ghoul/io/texture/texturewriterstb.h>

#include <ghoul/opengl/texture.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <filesystem>

namespace ghoul::io {

void TextureWriterSTB::saveTexture(const opengl::Texture& texture,
                                   const std::string& filename) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    std::string extension = std::filesystem::path(filename).extension().string();
    if (!extension.empty()) {
        extension = extension.substr(1);
    }
    ghoul_assert(!extension.empty(), "Filename must have an extension");

    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c) { return std::tolower(c); });

    int w = texture.width();
    int h = texture.height();

    int nComponents = texture.numberOfChannels();
    const void* data = texture.pixelData();

    // @TODO handle 3D textures! (And texture dimensions in general)
    int res = -1;
    if (extension == "jpeg" || extension == "jpg") {

    }
    else if (extension == "png") {
        res = stbi_write_png(filename.c_str(), w, h, nComponents, data, 0);
    }
    else if (extension == "bmp") {

    }
    else if (extension == "tga") {

    }
    else if (extension == "hdr") {

    }
    else {
        // @TODO: Just write to a png anyways?
        throw TextureWriteException(
            filename,
            fmt::format("Could not write to image with file extension {}", extension),
            *this
        );
    }
}

std::vector<std::string> TextureWriterSTB::supportedExtensions() const {
    // Taken from stb_image_writer.h
    return {
        "jpeg", "jpg",
        "png",
        "bmp",
        "tga",
        "hdr"
    };
}

} // namespace ghoul::io
