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

#include <ghoul/opengl/textureconversion.h>

#include <ghoul/misc/assert.h>
#include <cstring>
#include <vector>

namespace ghoul::opengl {

using ConversionFunc = void(*)(char* dst, const char* src, int nSourceChannels,
                               int nDestinationChannels, int nBytesPerChannel);

namespace {

template <Texture::Format from, Texture::Format to>
void convert(char* dst, const char* src, int nSourceChannels, int nDestinationChannels,
             int nBytesPerChannel)
{
    if (nSourceChannels <= nDestinationChannels) {
        std::memcpy(dst, src, nSourceChannels * nBytesPerChannel);
    }
    else {
        std::memcpy(dst, src, nDestinationChannels * nBytesPerChannel);
    }
}

template <>
void convert<Texture::Format::Red, Texture::Format::RG>(char* dst, const char* src,
                                                        int nSourceChannels,
                                                        int /*nDestinationChannels*/,
                                                        int nBytesPerChannel)
{
    int sourceSize = nSourceChannels * nBytesPerChannel;

    std::memcpy(dst, src, sourceSize);
    std::memcpy(dst + nBytesPerChannel, src, sourceSize);
}

template <>
void convert<Texture::Format::Red, Texture::Format::RGB>(char* dst, const char* src,
                                                         int /*nSourceChannels*/,
                                                         int /*nDestinationChannels*/,
                                                         int nBytesPerChannel)
{
    std::memcpy(dst, src, nBytesPerChannel);
    std::memcpy(dst + nBytesPerChannel, src, nBytesPerChannel);
    std::memcpy(dst + 2 * nBytesPerChannel, src, nBytesPerChannel);
}

template <>
void convert<Texture::Format::Red, Texture::Format::RGBA>(char* dst, const char* src,
                                                          int /*nSourceChannels*/,
                                                          int /*nDestinationChannels*/,
                                                          int nBytesPerChannel)
{
    std::memcpy(dst, src, nBytesPerChannel);
    std::memcpy(dst + nBytesPerChannel, src, nBytesPerChannel);
    std::memcpy(dst + 2 * nBytesPerChannel, src, nBytesPerChannel);
    std::memcpy(dst + 3 * nBytesPerChannel, src, nBytesPerChannel);
}

template <>
void convert<Texture::Format::RG, Texture::Format::RGBA>(char* dst, const char* src,
                                                        int /*nSourceChannels*/,
                                                        int /*nDestinationChannels*/,
                                                        int nBytesPerChannel)
{
    // Copying R into RGB
    std::memcpy(dst, src, nBytesPerChannel);
    std::memcpy(dst + nBytesPerChannel, src, nBytesPerChannel);
    std::memcpy(dst + 2 * nBytesPerChannel, src, nBytesPerChannel);

    // Copying B into A
    std::memcpy(dst + 3 * nBytesPerChannel, src + nBytesPerChannel, nBytesPerChannel);
}

ConversionFunc conversionFunctionSelector(Texture::Format from, Texture::Format to) {
    switch (from) {
        case Texture::Format::Red:
            switch (to) {
                case Texture::Format::RG:
                    return &convert<Texture::Format::Red, Texture::Format::RG>;
                case Texture::Format::RGB:
                    return &convert<Texture::Format::Red, Texture::Format::RGB>;
                case Texture::Format::RGBA:
                    return &convert<Texture::Format::Red, Texture::Format::RGBA>;
                default:
                    throw MissingCaseException();
            }
        case Texture::Format::RG:
            switch (to) {
                case Texture::Format::Red:
                    return &convert<Texture::Format::RG, Texture::Format::Red>;
                case Texture::Format::RGB:
                    return &convert<Texture::Format::RG, Texture::Format::RGB>;
                case Texture::Format::RGBA:
                    return &convert<Texture::Format::RG, Texture::Format::RGBA>;
                default:
                    throw MissingCaseException();
            }
        case Texture::Format::RGB:
            switch (to) {
                case Texture::Format::Red:
                    return &convert<Texture::Format::RGB, Texture::Format::Red>;
                case Texture::Format::RG:
                    return &convert<Texture::Format::RGB, Texture::Format::RG>;
                case Texture::Format::RGBA:
                    return &convert<Texture::Format::RGB, Texture::Format::RGBA>;
                default:
                    throw MissingCaseException();
            }
        case Texture::Format::RGBA:
            switch (to) {
                case Texture::Format::Red:
                    return &convert<Texture::Format::RGBA, Texture::Format::Red>;
                case Texture::Format::RG:
                    return &convert<Texture::Format::RGBA, Texture::Format::RG>;
                case Texture::Format::RGB:
                    return &convert<Texture::Format::RGBA, Texture::Format::RGB>;
                default:
                    throw MissingCaseException();
            }
        default:
            throw MissingCaseException();
    }
}

} // namespace

void convertTextureFormat(Texture& texture, Texture::Format newFormat) {
    ghoul_assert(
        texture.dataOwnership(),
        "Texture must have ownership of its data for successful conversion"
    );

    if (newFormat == texture.format()) {
        return;
    }

    int nPixels = glm::compMul(texture.dimensions());
    int bytesPerChannel = texture.bytesPerPixel() / texture.numberOfChannels();

    int nChannelsOld = texture.numberOfChannels();
    int nChannelsNew = Texture::numberOfChannels(newFormat);

    ConversionFunc convFunc = conversionFunctionSelector(texture.format(), newFormat);

    const char* oldData = reinterpret_cast<const char*>(texture.pixelData());
    char* newData = new char[nChannelsNew * bytesPerChannel * nPixels];
    std::memset(newData, 0, nChannelsNew * bytesPerChannel * nPixels);

    // Conversion
    for (int i = 0; i < nPixels; ++i) {
        const char* oldPtr = oldData + i * bytesPerChannel * nChannelsOld;
        char* newPtr = newData + i * bytesPerChannel * nChannelsNew;

        convFunc(newPtr, oldPtr, nChannelsOld, nChannelsNew, bytesPerChannel);
    }

    texture.setFormat(newFormat);
    texture.setInternalFormat(static_cast<GLenum>(newFormat));

    texture.setPixelData(newData);
}

} // namespace ghoul::opengl
