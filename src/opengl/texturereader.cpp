/**************************************************************************************************
 * GHOUL                                                                                          *
 * General Helpful Open Utility Library                                                           *
 *                                                                                                *
 * Copyright (c) 2012 Alexander Bock                                                              *
 *                                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software  *
 * and associated documentation files (the "Software"), to deal in the Software without           *
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the  *
 * Software is furnished to do so, subject to the following conditions:                           *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all copies or       *
 * substantial portions of the Software.                                                          *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  *
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 *************************************************************************************************/

#include <ghoul/opengl/texturereader.h>
#include <ghoul/opengl/texture.h>
#include <ghoul/logging/logmanager.h>
#include "glm/glm.hpp"

#include <IL/il.h>

namespace ghoul {
namespace opengl {

Texture* loadTexture(const std::string& filename, bool useCompression) {
    std::string _loggerCat = "loadTexture";

    ilInit();
    //ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    //ilEnable(IL_ORIGIN_SET);

    ILboolean loadSuccess = ilLoadImage(filename.c_str());
    if (!loadSuccess) {
        ILenum error = ilGetError();
        if (error == IL_COULD_NOT_OPEN_FILE)
            LERROR_SAFE("Could not open image file '" << filename << "'. File did not exist.");
        else if (error == IL_ILLEGAL_OPERATION)
            LERROR_SAFE("Could not open image file '" << filename << "'. Illegal operation.");
        else if (error == IL_INVALID_EXTENSION)
            LERROR_SAFE("Could not open image file '" << filename << "'. The file could not be loaded based on extension or header.");
        else if (error == IL_INVALID_PARAM)
            LERROR_SAFE("Could not open image file '" << filename << "'. FileName was not valid. It was most likely NULL.");
        else if (error == IL_OUT_OF_MEMORY)
            LERROR_SAFE("Could not allocate enough memory for '" << filename << "'.");
        else if (error == IL_NO_ERROR)
            LERROR_SAFE("Could not open image file '" << filename << "'. No error reported.");
        else if (error == IL_UNKNOWN_ERROR)
            LERROR_SAFE("Could not open image file '" << filename << "'. Unknown reported.");
        else
            LERROR_SAFE("Could not open/read image file '" << filename << "'. Error message: " << error);
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
            format = Texture::FormatRGB;
            break;
        case IL_RGBA:
            format = Texture::FormatRGBA;
            break;
        case IL_BGR:
            format = Texture::FormatBGR;
            break;
        case IL_BGRA:
            format = Texture::FormatBGRA;
            break;
        default:
            LERROR_SAFE("Could not read image file '" << filename << "' of format: '" << imageFormat << "'");
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
            LERROR_SAFE("Could not read image file '" << filename << "' of data type: '" << imageType << "'");
            return nullptr;
    }

    Texture* result;
    if (useCompression) {
        switch (format) {
            case Texture::FormatRGB:
                result = new Texture(data, size, format, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, type);
                break;
            case Texture::FormatRGBA:
                result = new Texture(data, size, format, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, type);
                break;
            default:
                LERROR_SAFE("Could not assign compressed format for: '" << format <<
                    "'. Using no compression instead");
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
