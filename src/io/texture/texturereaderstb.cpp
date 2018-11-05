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

#include <ghoul/io/texture/texturereaderstb.h>

#ifdef GHOUL_USE_STB_IMAGE

#include <ghoul/fmt.h>
#include <ghoul/glm.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/opengl/texture.h>
#include <algorithm>
#include <stb_image.h>
#include <iostream>

namespace {
    std::unique_ptr<ghoul::opengl::Texture> load(unsigned char* data, int x, int y,
                                                 int n, const std::string& message,
                                                 const ghoul::io::TextureReaderBase* r)
    {
        if (!data) {
            throw ghoul::io::TextureReaderBase::TextureLoadException(
                message,
                fmt::format("Error reading image data: {}", stbi_failure_reason()),
                r
            );
        }


        LDEBUGC("TextureReaderSTB", fmt::format("{}: {} {} {}\n", message, x, y, n));

        // This is weird.  stb_image.h says that the first pixel loaded is the one in the
        // upper left.  However, if we load the data and just use it, the images are
        // flipped in y.  I assume that there is a 1-t floating around somewhere and
        // someone should hunt that down.  For now, we just flip these values as well.
        // In addition, stb uses malloc to clear the memory and since we use delete[] for
        // cleanup in the texture class, I don't know what will happen. But probably
        // nothing good, so we copy it into a new array (abock)

        unsigned char* newData = new unsigned char[x * y * n];
        std::memset(newData, 255, x * y * n);

        // As we only need to flip in y direction, we can take entire scanlines and move
        // the ith line to the (y-i-1)th line
        for (int i = 0; i < y; ++i) {
            std::memmove(newData + (i * x * n), data + ((y-i-1) * x * n), x * n);
        }
        // We don't need the original data anymore, so we can free it
        stbi_image_free(data);

        ghoul::opengl::Texture::Format format;
        GLenum internalFormat;
        switch (n) {
            case 1:
                format = ghoul::opengl::Texture::Format::Red;
                internalFormat = GL_RED;
                break;
            case 2:
                format = ghoul::opengl::Texture::Format::RG;
                internalFormat = GL_RG;
                break;
            case 3:
                format = ghoul::opengl::Texture::Format::RGB;
                internalFormat = GL_RGB;
                break;
            case 4:
                format = ghoul::opengl::Texture::Format::RGBA;
                internalFormat = GL_RGBA;
                break;
        }

        GLenum type = GL_UNSIGNED_BYTE;

        return std::make_unique<ghoul::opengl::Texture>(
            newData,
            glm::uvec3(x, y, 1),
            format,
            internalFormat,
            type
        );
    }
} // namespace

namespace ghoul::io {

std::unique_ptr<opengl::Texture> TextureReaderSTB::loadTexture(
                                                        const std::string& filename) const
{
    int x;
    int y;
    int n;
    unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, 0);

    return load(data, x, y, n, filename, this);

}

std::unique_ptr<opengl::Texture> TextureReaderSTB::loadTexture(void* memory,
                                                                 size_t size) const
{
    int x;
    int y;
    int n;
    unsigned char* data = stbi_load_from_memory(
        reinterpret_cast<const unsigned char*>(memory),
        static_cast<int>(size),
        &x,
        &y,
        &n,
        0
    );

    return load(data, x, y, n, "Memory", this);

}

std::vector<std::string> TextureReaderSTB::supportedExtensions() const {
    // Taken from stb_image.h
    return {
        "jpeg", "jpg",
        "png",
        "bmp",
        "psd",  // Photoshop
        "tga",
        "gif",
        "hdr",  // Radiance RGBE format
        "pic",  // Softimage PIC
        "ppm", "pgm"
    };
}

} // namespace ghoul::io

#endif // GHOUL_USE_STB_IMAGE
