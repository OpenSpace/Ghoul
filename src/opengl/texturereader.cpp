/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
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
#include <ghoul/glm.h>

#if defined( GHOUL_USE_DEVIL )
#include <IL/il.h>
#include <IL/ilu.h>
#elif defined( GHOUL_USE_FREEIMAGE)
#define FREEIMAGE_COLORORDER_BGR 0
#define FREEIMAGE_COLORORDER_RGB 1
#if defined(APPLE) || defined(FREEIMAGE_BIGENDIAN)
#define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_RGB
#else
#define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_RGB
#endif
#include <FreeImage.h>
#endif

namespace ghoul {
namespace opengl {

Texture* loadTexture(const std::string& filename, bool useCompression) {
    std::string _loggerCat = "loadTexture";

#if defined( GHOUL_USE_DEVIL )
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

#elif defined( GHOUL_USE_FREEIMAGE)

	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height
	unsigned int width(0), height(0);
	//OpenGL's image ID to map to
	GLuint gl_texID;

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(filename.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filename.c_str());
	//if still unkown, return failure
	if (fif == FIF_UNKNOWN)
		return nullptr;

	//check that the plugin has reading capabilities and load the file
	if (FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, filename.c_str());
	//if the image failed to load, return failure
	if (!dib)
		return nullptr;

	//retrieve the image data
	bits = FreeImage_GetBits(dib);
	//get the image width and height
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);

	glm::size3_t size(width, height, 1);
	//if this somehow one of these failed (they shouldn't), return failure
	if ((bits == 0) || (width == 0) || (height == 0))
		return nullptr;


	FREE_IMAGE_TYPE			imageType = FreeImage_GetImageType(dib);
	FREE_IMAGE_COLOR_TYPE	colorType = FreeImage_GetColorType(dib);
	BITMAPINFOHEADER*		infoheader = FreeImage_GetInfoHeader(dib);
	/*
	FIT_UNKNOWN = 0,	// unknown type
		FIT_BITMAP = 1,	// standard image			: 1-, 4-, 8-, 16-, 24-, 32-bit
		FIT_UINT16 = 2,	// array of unsigned short	: unsigned 16-bit
		FIT_INT16 = 3,	// array of short			: signed 16-bit
		FIT_UINT32 = 4,	// array of unsigned long	: unsigned 32-bit
		FIT_INT32 = 5,	// array of long			: signed 32-bit
		FIT_FLOAT = 6,	// array of float			: 32-bit IEEE floating point
		FIT_DOUBLE = 7,	// array of double			: 64-bit IEEE floating point
		FIT_COMPLEX = 8,	// array of FICOMPLEX		: 2 x 64-bit IEEE floating point
		FIT_RGB16 = 9,	// 48-bit RGB image			: 3 x 16-bit
		FIT_RGBA16 = 10,	// 64-bit RGBA image		: 4 x 16-bit
		FIT_RGBF = 11,	// 96-bit RGB float image	: 3 x 32-bit IEEE floating point
		FIT_RGBAF = 12	// 128-bit RGBA float image	: 4 x 32-bit IEEE floating point

		FI_ENUM(FREE_IMAGE_COLOR_TYPE) {
		FIC_MINISWHITE = 0,		// min value is white
		FIC_MINISBLACK = 1,		// min value is black
		FIC_RGB        = 2,		// RGB color model
		FIC_PALETTE    = 3,		// color map indexed
		FIC_RGBALPHA   = 4,		// RGB color model with alpha channel
		FIC_CMYK       = 5		// CMYK color model
		};

		*/

	if (imageType != FIT_BITMAP) {
		LERROR("Could not read image file '" << filename <<
			"' of data type: '" << imageType << "'");
		return nullptr;
	}
	GLenum type = GL_UNSIGNED_BYTE;
	Texture::Format format;
	switch (colorType) {
	case FIC_RGB:
		format = Texture::Format::RGB;
		break;
	case FIC_RGBALPHA:
		format = Texture::Format::RGBA;
		break;
	default:
		LERROR("Could not read image file '" << filename <<
			"' of color type: '" << colorType << "'");
		return nullptr;

	}
	int imageByte = FreeImage_GetBPP(dib);
	unsigned int pitch = FreeImage_GetPitch(dib);
	BYTE* data = new BYTE[width * height * imageByte/8];

	FreeImage_ConvertToRawBits(data, dib, pitch, imageByte, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
	FreeImage_Unload(dib);


	// Swap red and blue channels, cannot use GL_BGR in OpenGL core profile
	for (size_t i = 0; i < width * height; ++i) {
		size_t index = i * imageByte / 8;
		std::swap(data[index], data[index + 2]);
	}

#endif

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
