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

#include <ghoul/io/texture/texturereadercmap.h>

#include <ghoul/misc/assert.h>
#include <ghoul/opengl/texture.h>
#include <ghoul/glm.h>

#include <cppformat/format.h>

#include <fstream>
#include <sstream>

namespace ghoul {
namespace io {

std::unique_ptr<opengl::Texture> TextureReaderCMAP::loadTexture(
                                                               std::string filename) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");
    
    std::ifstream file;
    file.exceptions(std::ifstream::failbit);
    file.open(filename, std::ifstream::in);
    file.exceptions(std::ifstream::goodbit);

    int width = 0;
    uint8_t* values = nullptr;
//    std::vector<uint8_t> values;

    std::string line;
    int i = 0;
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty() || line == "\r")
            continue;
        // # defines a comment
        if (line[0] == '#')
            continue;

        std::stringstream s(line);
        if (!values) {
            s >> width;
            values = new uint8_t[width * 4];
            continue;
        }
        
        if (!values) {
            throw TextureLoadException(
                std::move(filename),
                "The first non-comment, non-empty line must contain the image width",
                this
            );
        }

        glm::vec4 color;
        s >> color.r;
        s >> color.g;
        s >> color.b;
        s >> color.a;
        
        if (i > (width * 4)) {
            throw TextureLoadException(
                std::move(filename),
                fmt::format("Header assured '{}' values but more were found", width),
                this
            );
        }

        values[i++] = static_cast<uint8_t>(color.r * 255);
        values[i++] = static_cast<uint8_t>(color.g * 255);
        values[i++] = static_cast<uint8_t>(color.b * 255);
        values[i++] = static_cast<uint8_t>(color.a * 255);
    }

    if ((width * 4) != i) {
        throw TextureLoadException(
            std::move(filename),
            fmt::format("Header assured '{}' values but '{}' were found", width, i / 4.f),
            this
        );
    }

    using opengl::Texture;
    std::unique_ptr<Texture> texture = std::make_unique<Texture>(
        values,                         // data
        glm::size3_t(width, 1, 1),        // dimensions
        Texture::Format::RGBA            // Format
    );

    return texture;
}

std::unique_ptr<opengl::Texture> TextureReaderCMAP::loadTextureFromMemory(const std::string& buffer) const {
    return nullptr;
}

std::vector<std::string> TextureReaderCMAP::supportedExtensions() const {
    return { "cmap" };
}

} // namespace io
} // namespace ghoul
