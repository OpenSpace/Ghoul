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

#include <ghoul/io/texture/texturereadersoil.h>

#ifdef GHOUL_USE_SOIL

#include <ghoul/opengl/texture.h>
#include <ghoul/glm.h>

#include <SOIL.h>

namespace ghoul {
namespace io {

std::unique_ptr<opengl::Texture> TextureReaderSOIL::loadTexture(
                                                        const std::string& filename) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");
    
    using opengl::Texture;
 
    int width, height;
    unsigned char* image = SOIL_load_image(
        filename.c_str(),
        &width,
        &height,
        0,
        SOIL_LOAD_RGBA
    );

    glm::size3_t size(width, height, 1);
    Texture::Format format;
    format = Texture::Format::RGBA;
    GLenum type;
    type = GL_UNSIGNED_BYTE;

    return std::make_unique<Texture>(
        image,
        size,
        format,
        static_cast<int>(format),
        type,
        Texture::FilterMode::Linear
    );
}

std::unique_ptr<opengl::Texture> TextureReaderSOIL::loadTexture(void* memory,
                                                                size_t size) const
{
    ghoul_assert(!buffer.empty(), "Buffer must not be empty");
    
    using opengl::Texture;
 
    int width, height;
    unsigned char* image = SOIL_load_image_from_memory(
        reinterpret_cast<unsigned char*>(memory),
        size,
        &width,
        &height,
        0,
        SOIL_LOAD_RGBA
    );

    glm::size3_t imageSize(width, height, 1);
    Texture::Format format;
    format = Texture::Format::RGBA;
    GLenum type;
    type = GL_UNSIGNED_BYTE;

    return std::make_unique<Texture>(
        image,
        imageSize,
        format,
        static_cast<int>(format),
        type,
        Texture::FilterMode::Linear
    );
}

std::vector<std::string> TextureReaderSOIL::supportedExtensions() const {
    // taken from http://www.lonesock.net/soil.html
    return {
        "bmp",    // load & save                    
        "png",  // load
        "jpg",  // load
        "jpeg",
        "tga",  // load & save
        "dds",  // load & save
        "psd",
        "hdr",

    // BMP - non-1bpp, non-RLE (from stb_image documentation)
    // PNG - non-interlaced (from stb_image documentation)
    // JPG - JPEG baseline (from stb_image documentation)
    // TGA - greyscale or RGB or RGBA or indexed, uncompressed or RLE
    // DDS - DXT1/2/3/4/5, uncompressed, cubemaps (can't read 3D DDS files yet)
    // PSD - (from stb_image documentation)
    // HDR - converted to LDR, unless loaded with *HDR* functions (RGBE or RGBdivA or RGBdivA2) 
    };
}

} // namespace opengl
} // namespace ghoul

#endif // GHOUL_USE_SOIL
