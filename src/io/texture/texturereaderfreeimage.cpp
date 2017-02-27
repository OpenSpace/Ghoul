/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#include <ghoul/io/texture/texturereaderfreeimage.h>

#ifdef GHOUL_USE_FREEIMAGE

#include <ghoul/opengl/texture.h>
#include <ghoul/glm.h>

#define FREEIMAGE_COLORORDER_BGR 0
#define FREEIMAGE_COLORORDER_RGB 1
#if defined(APPLE) || defined(FREEIMAGE_BIGENDIAN)
#define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_BGR
#else
#define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_RGB
#endif
#include <FreeImage.h>


#include <iostream>
namespace ghoul {
namespace io {

std::unique_ptr<opengl::Texture> TextureReaderFreeImage::loadTextureInternal(
    const std::string& source, FIBITMAP* dib) const
{
    //retrieve the image data
    BYTE* bits = FreeImage_GetBits(dib);
    //get the image width and height
    unsigned int width = FreeImage_GetWidth(dib);
    unsigned int height = FreeImage_GetHeight(dib);

    glm::size3_t imageSize(width, height, 1);
    //if this somehow one of these failed (they shouldn't), return failure
    if ((bits == 0) || (width == 0) || (height == 0))
        throw TextureLoadException("Memory", "Unable to ready bits or size", this);


    FREE_IMAGE_TYPE            imageType = FreeImage_GetImageType(dib);
    FREE_IMAGE_COLOR_TYPE    colorType = FreeImage_GetColorType(dib);
    //BITMAPINFOHEADER*        infoheader = FreeImage_GetInfoHeader(dib);
    /*
    FIT_UNKNOWN = 0,    // unknown type
    FIT_BITMAP = 1,    // standard image            : 1-, 4-, 8-, 16-, 24-, 32-bit
    FIT_UINT16 = 2,    // array of unsigned short    : unsigned 16-bit
    FIT_INT16 = 3,    // array of short            : signed 16-bit
    FIT_UINT32 = 4,    // array of unsigned long    : unsigned 32-bit
    FIT_INT32 = 5,    // array of long            : signed 32-bit
    FIT_FLOAT = 6,    // array of float            : 32-bit IEEE floating point
    FIT_DOUBLE = 7,    // array of double            : 64-bit IEEE floating point
    FIT_COMPLEX = 8,    // array of FICOMPLEX        : 2 x 64-bit IEEE floating point
    FIT_RGB16 = 9,    // 48-bit RGB image            : 3 x 16-bit
    FIT_RGBA16 = 10,    // 64-bit RGBA image        : 4 x 16-bit
    FIT_RGBF = 11,    // 96-bit RGB float image    : 3 x 32-bit IEEE floating point
    FIT_RGBAF = 12    // 128-bit RGBA float image    : 4 x 32-bit IEEE floating point

    FI_ENUM(FREE_IMAGE_COLOR_TYPE) {
    FIC_MINISWHITE = 0,        // min value is white
    FIC_MINISBLACK = 1,        // min value is black
    FIC_RGB        = 2,        // RGB color model
    FIC_PALETTE    = 3,        // color map indexed
    FIC_RGBALPHA   = 4,        // RGB color model with alpha channel
    FIC_CMYK       = 5        // CMYK color model
    };

    */

    if (imageType != FIT_BITMAP)
        throw TextureLoadException("Memory", "Could not read image", this);

    if (colorType == FIC_MINISBLACK || colorType == FIC_PALETTE) {
        FIBITMAP* tempImage = dib;
        dib = FreeImage_ConvertTo32Bits(tempImage);
        FreeImage_Unload(tempImage);
    }

    int imageByte = FreeImage_GetBPP(dib);

    GLenum type = GL_UNSIGNED_BYTE;
    opengl::Texture::Format format;
    switch (imageByte) {
        case 24:
            format = opengl::Texture::Format::RGB;
            break;
        case 32:
            format = opengl::Texture::Format::RGBA;
            break;
        default:
            throw TextureLoadException("Memory", "Could not read image", this);
    }

    unsigned int pitch = FreeImage_GetPitch(dib);
    BYTE* data = new BYTE[height * pitch];

    // Swap red and blue channels, cannot use GL_BGR in OpenGL core profile
    for (unsigned y = 0; y < FreeImage_GetHeight(dib); y++) {
        BYTE* bits = FreeImage_GetScanLine(dib, y);
        for (unsigned x = 0; x < FreeImage_GetWidth(dib); x++) {
            std::swap(bits[FI_RGBA_RED], bits[FI_RGBA_BLUE]);
            // jump to next pixel
            bits += imageByte / 8;
        }
    }

    bool flipSuccess = FreeImage_FlipVertical(dib);
    if (!flipSuccess) {
        throw TextureLoadException("Memory", "Could not flip image", this);
    }

    FreeImage_ConvertToRawBits(
        data,
        dib,
        pitch,
        imageByte,
        FI_RGBA_RED_MASK,
        FI_RGBA_GREEN_MASK,
        FI_RGBA_BLUE_MASK,
        TRUE
    );
    FreeImage_Unload(dib);

    return std::make_unique<opengl::Texture>(
        data,
        imageSize,
        format,
        static_cast<int>(format),
        type
        );
}

std::unique_ptr<opengl::Texture> TextureReaderFreeImage::loadTexture(
                                                        const std::string& filename) const
{
    // Pointer to the image, once loaded
    FIBITMAP* dib(0);

    // Image format
    // Check the file signature and deduce its format
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename.c_str(), 0);
    // If still unknown, try to guess the file format from the file extension
    if (fif == FIF_UNKNOWN) {
        fif = FreeImage_GetFIFFromFilename(filename.c_str());
    }
    // If still unknown, return failure
    if (fif == FIF_UNKNOWN) {
        throw TextureLoadException(filename, "Could not determine file format", this);
    }

    // Check that the plug-in has reading capabilities and load the file
    if (FreeImage_FIFSupportsReading(fif)) {
        dib = FreeImage_Load(fif, filename.c_str());
    }
    // If the image failed to load, return failure
    if (!dib) {
        throw TextureLoadException(filename, "Could not load image", this);
    }

    return loadTextureInternal(filename, dib);
}

std::unique_ptr<opengl::Texture> TextureReaderFreeImage::loadTexture(void* memory,
                                                                     size_t size) const
{
    using opengl::Texture;

    // Pointer to the image, once loaded
    FIBITMAP* dib(0);

    // Open the a stream to memory
    FIMEMORY* stream = FreeImage_OpenMemory(
        reinterpret_cast<BYTE*>(memory),
        size
    );

    // Check the file signature and deduce its format
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(stream, 0);
    //if it is unknown, return failure
    if (fif == FIF_UNKNOWN) {
        throw TextureLoadException("Memory", "Could not deduce type information", this);
    }

    //check that the plug-in has reading capabilities and load the file
    if (FreeImage_FIFSupportsReading(fif)) {
        dib = FreeImage_LoadFromMemory(fif, stream);
    }
    //if the image failed to load, return failure
    if (!dib) {
        throw TextureLoadException("Memory", "Could not load image", this);
    }

    //close the memory stream
    FreeImage_CloseMemory(stream);

    return loadTextureInternal("Memory", dib);
}

std::vector<std::string> TextureReaderFreeImage::supportedExtensions() const {
    // Taken from http://freeimage.sourceforge.net/features.html
    return {
        "bmp",
        "cut",
        "dds",
        "exr",
        "g3",
        "gif",
        "hdr",
        "ico",
        "iff",
        "jbig",
        "jng",
        "jpg", "jpeg",
        "koala",
        "mng"
        "pcx",
        "pbm", "pgm", "ppm",
        "pfm",
        "png",
        "pict",
        "psd",
        "raw",
        "ras",
        "sgi",
        "targa",
        "tif", "tiff",
        "wbmp",
        "webp",
        "xbm",
        "xpm"
    };
}

} // namespace opengl
} // namespace ghoul

#endif // GHOUL_USE_DEVIL
