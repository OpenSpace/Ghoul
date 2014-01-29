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

#include <ghoul/opengl/texturereader.h>

#include <ghoul/opengl/texture.h>
#include <ghoul/logging/logmanager.h>
#include "glm/glm.hpp"

#include <IL/il.h>
#include <IL/ilu.h>

namespace ghoul {
namespace opengl {

Texture* loadTexture(const std::string& filename, bool useCompression) {
    std::string _loggerCat = "loadTexture";

    ilInit();
    iluInit();
    
    //ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    //ilEnable(IL_ORIGIN_SET);

    ILboolean loadSuccess = ilLoadImage(filename.c_str());
    if (!loadSuccess) {
        ILenum error = ilGetError();
        LERROR("Error while loading image '" << filename << "': " <<
                    iluErrorString(error));
        return nullptr;
    }
    ILint imageFormat = ilGetInteger(IL_IMAGE_FORMAT);
    ILint imageType = ilGetInteger(IL_IMAGE_TYPE);
    ILint imageByte = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
    ILint width = ilGetInteger(IL_IMAGE_WIDTH);
    ILint height = ilGetInteger(IL_IMAGE_HEIGHT);
    ILint depth = ilGetInteger(IL_IMAGE_DEPTH);

    // Copy data from common data store to own address space
    ILubyte* data = new ILubyte[width * height * imageByte];
    ilCopyPixels(0, 0, 0, width, height, 1, imageFormat, IL_UNSIGNED_BYTE, data);

    glm::size3_t size(width, height, depth);

    Texture::Format format;
    switch (imageFormat) {
        case IL_RGB:
            format = Texture::Format::RGB;
            break;
        case IL_RGBA:
            format = Texture::Format::RGBA;
            break;
        case IL_BGR:
            format = Texture::Format::BGR;
            break;
        case IL_BGRA:
            format = Texture::Format::BGRA;
            break;
        default:
            LERROR("Could not read image file '" << filename << "' of format: '" <<
                        imageFormat << "'");
            return nullptr;
    }

    
    GLenum type;
    switch (imageType) {
        case IL_UNSIGNED_BYTE:
            type = GL_UNSIGNED_BYTE;
            break;
        case IL_BYTE:
            type = GL_BYTE;
            break;
        case IL_UNSIGNED_SHORT:
            type = GL_UNSIGNED_SHORT;
            break;
        case IL_SHORT:
            type = GL_SHORT;
            break;
        case IL_UNSIGNED_INT:
            type = GL_UNSIGNED_INT;
            break;
        case IL_INT:
            type = GL_INT;
            break;
        case IL_FLOAT:
            type = GL_FLOAT;
            break;
        default:
            LERROR("Could not read image file '" << filename <<
                        "' of data type: '" << imageType << "'");
            return nullptr;
    }

    Texture* result;
    if (useCompression) {
        switch (format) {
            case Texture::Format::RGB:
                result = new Texture(data, size, format,
                                     GL_COMPRESSED_RGB_S3TC_DXT1_EXT, type);
                break;
            case Texture::Format::RGBA:
                result = new Texture(data, size, format,
                                     GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, type);
                break;
            default:
                LERROR("Could not assign compressed format for: '" <<
                    GLint(format) << "'. Using no compression instead");
                result = new Texture(data, size, format, static_cast<int>(format), type);
                break;
        }
    }
    else
        result = new Texture(data, size, format, static_cast<int>(format), type);

    return result;
}

} // namespace opengl
} // namespace ghoul
