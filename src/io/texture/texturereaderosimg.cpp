/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2025                                                               *
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

#include <ghoul/io/texture/texturereaderosimg.h>

#include <ghoul/format.h>
#include <ghoul/glm.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/stringhelper.h>
#include <ghoul/opengl/texture.h>
#include <fstream>
#include <sstream>

namespace {
    constexpr int8_t CurrentMajorVersion = 0;
    constexpr int8_t CurrentMinorVersion = 1;
}

namespace ghoul::io {

std::unique_ptr<opengl::Texture> TextureReaderOsImg::loadTexture(
                                                    const std::filesystem::path& filename,
                                                                    int nDimensions) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    if (nDimensions != 2) {
        throw ghoul::RuntimeError(std::format(
            "The number of dimensions for '{}' must be 2, but was {}",
            filename, nDimensions
        ));
    }

    std::ifstream fileStream(filename, std::ifstream::binary);
    if (!fileStream.good()) {
        throw ghoul::RuntimeError(std::format("Could not open OS image file {}", filename));
    }

    // First read the header
    // Check the file format version
    int8_t majorVersion = 0;
    int8_t minorVersion = 0;
    fileStream.read(reinterpret_cast<char*>(&majorVersion), sizeof(uint8_t));
    fileStream.read(reinterpret_cast<char*>(&minorVersion), sizeof(uint8_t));
    if (majorVersion != CurrentMajorVersion || minorVersion != CurrentMinorVersion)
    {
        throw ghoul::RuntimeError(
            std::format("OS img format {}.{} is not supported in file {}", majorVersion,
                minorVersion, filename
            )
        );
    }

    // Resolution
    int width = 0;
    fileStream.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));
    int height = 0;
    fileStream.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));

    // TODO: Read the number of channels

    // Min and max value
    float minValue = 0.f;
    fileStream.read(reinterpret_cast<char*>(&minValue), sizeof(float));
    float maxValue = 0.f;
    fileStream.read(reinterpret_cast<char*>(&maxValue), sizeof(float));

    // Now read the pixel data
    const size_t nPixels = static_cast<size_t>(width) * static_cast<size_t>(height);
    std::vector<float> values(nPixels);
    fileStream.read(reinterpret_cast<char*>(values.data()), nPixels * sizeof(float));

    // Scale the pixel values using the min and max values
    const float scale = maxValue - minValue;
    for (float& v : values) {
        v = minValue + v / scale;
    }

    return std::make_unique<opengl::Texture>(
        values.data(),
        glm::size3_t(width, height, 1),
        GL_TEXTURE_2D,
        opengl::Texture::Format::Red,
        GL_RED,
        GL_FLOAT
    );
}

std::unique_ptr<opengl::Texture> TextureReaderOsImg::loadTexture(void*, size_t, int) const
{
    ghoul_assert(false, "Implementation missing");
    return nullptr;
}

std::vector<std::string> TextureReaderOsImg::supportedExtensions() const {
    return { "osimg" };
}

} // namespace ghoul::io
