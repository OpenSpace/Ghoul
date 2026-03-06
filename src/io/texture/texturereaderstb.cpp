/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2026                                                               *
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

#include <ghoul/format.h>
#include <ghoul/glm.h>
#include <ghoul/opengl/texture.h>
#include <stb_image.h>
#include <algorithm>
#include <cstring>

namespace {
    using namespace ghoul;
    using namespace ghoul::io;

    std::unique_ptr<opengl::Texture> load(unsigned char* data, int x, int y, int n,
                                          const std::string& message,
                                          const TextureReaderBase* r, int nDimensions,
                                          opengl::Texture::SamplerInit samplerSettings)
    {
        if (!data) {
            throw TextureReaderBase::TextureLoadException(
                message,
                std::format("Error reading image data: {}", stbi_failure_reason()),
                r
            );
        }


        // This is weird. stb_image.h says that the first pixel loaded is the one in the
        // upper left. However, if we load the data and just use it, the images are
        // flipped in y. I assume that there is a 1-t floating around somewhere and
        // someone should hunt that down.  For now, we just flip these values as well

        // As we only need to flip in y direction, we can take entire scanlines and move
        // the ith line to the (y-i-1)th line
        std::vector<std::byte> buffer = std::vector<std::byte>(x * n);
        for (int i = 0; i < y / 2; i++) {
            // 1. Save the destination
            std::memcpy(buffer.data(), data + (i * x * n), x * n);

            // 2. Copy the source to destination
            std::memcpy(data + (i * x * n), data + (y - i - 1) * x * n, x * n);

            // 3. Copy into the source
            std::memcpy(data + (y - i - 1) * x * n, buffer.data(), x * n);
        }

        const opengl::Texture::Format format = [](int nDim) {
            switch (nDim) {
                case 1: return opengl::Texture::Format::Red;
                case 2: return opengl::Texture::Format::RG;
                case 3: return opengl::Texture::Format::RGB;
                case 4: return opengl::Texture::Format::RGBA;
                default:
                    throw RuntimeError(std::format("Unknown dimension '{}'", nDim));
            }
        }(n);
        const GLenum type = [](int d) {
            switch (d) {
                case 1: return GL_TEXTURE_1D;
                case 2: return GL_TEXTURE_2D;
                case 3: return GL_TEXTURE_3D;
                default:
                    throw RuntimeError(std::format("Unsupported dimensionality '{}'", d));
            }
        }(nDimensions);

        std::unique_ptr<opengl::Texture> texture =
            std::make_unique<opengl::Texture>(
                opengl::Texture::FormatInit{
                    .dimensions = glm::uvec3(x, y, 1),
                    .type = type,
                    .format = format,
                    .dataType = GL_UNSIGNED_BYTE
                },
                samplerSettings,
                reinterpret_cast<std::byte*>(data)
            );
        stbi_image_free(data);
        return texture;
    }
} // namespace

namespace ghoul::io {

std::unique_ptr<opengl::Texture> TextureReaderSTB::loadTexture(
                                                    const std::filesystem::path& filename,
                                                                          int nDimensions,
                                       opengl::Texture::SamplerInit samplerSettings) const
{
    const std::string f = filename.string();
    int x = 0;
    int y = 0;
    int n = 0;
    unsigned char* data = stbi_load(f.c_str(), &x, &y, &n, 0);

    return load(data, x, y, n, f, this, nDimensions, std::move(samplerSettings));
}

std::unique_ptr<opengl::Texture> TextureReaderSTB::loadTexture(void* memory, size_t size,
                                                               int nDimensions,
                                       opengl::Texture::SamplerInit samplerSettings) const
{
    int x = 0;
    int y = 0;
    int n = 0;
    unsigned char* data = stbi_load_from_memory(
        reinterpret_cast<unsigned char*>(memory),
        static_cast<int>(size),
        &x,
        &y,
        &n,
        0
    );

    return load(data, x, y, n, "Memory", this, nDimensions, std::move(samplerSettings));
}

glm::ivec2 TextureReaderSTB::imageSize(const std::filesystem::path& filename) const {
    const std::string f = filename.string();
    int x = 0;
    int y = 0;
    int n = 0;
    stbi_load(f.c_str(), &x, &y, &n, 0);

    return glm::ivec2(x, y);
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
