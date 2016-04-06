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

std::unique_ptr<opengl::Texture> TextureReaderFreeImage::loadTexture(
                                                               std::string filename) const
{
	using opengl::Texture;

	//pointer to the image, once loaded
	FIBITMAP* dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height
	unsigned int width(0), height(0);

    //image format
    //check the file signature and deduce its format
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filename.c_str());
	//if still unknown, return failure
	if (fif == FIF_UNKNOWN)
		return nullptr;

	//check that the plug-in has reading capabilities and load the file
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
	//BITMAPINFOHEADER*		infoheader = FreeImage_GetInfoHeader(dib);
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

	if (imageType != FIT_BITMAP)
        throw TextureLoadException(std::move(filename), "Could not read image", this);
    
    if (colorType == FIC_MINISBLACK || colorType == FIC_PALETTE) {
        colorType = FIC_RGB;
        dib = FreeImage_ConvertTo24Bits(dib);
//        dib = FreeImage_ConvertToRGBF(dib);
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
        throw TextureLoadException(std::move(filename), "Could not read image", this);
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

    return std::make_unique<Texture>(data, size, format, static_cast<int>(format), type);
}

std::unique_ptr<opengl::Texture> TextureReaderFreeImage::loadTextureFromMemory(const std::string& buffer) const {
	using opengl::Texture;

	//pointer to the image, once loaded
	FIBITMAP* dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height
	unsigned int width(0), height(0);

	//Open the a stream to memory
	FIMEMORY* stream = FreeImage_OpenMemory((BYTE*)buffer.c_str(), buffer.size());

	//check the file signature and deduce its format
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(stream, 0);
	//if it is unknown, return failure
	if (fif == FIF_UNKNOWN)
		return nullptr;

	//check that the plug-in has reading capabilities and load the file
	if (FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_LoadFromMemory(fif, stream);
	//if the image failed to load, return failure
	if (!dib)
		return nullptr;

	//close the memory stream
	FreeImage_CloseMemory(stream);

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
	//BITMAPINFOHEADER*		infoheader = FreeImage_GetInfoHeader(dib);
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

	if (imageType != FIT_BITMAP)
        throw TextureLoadException(std::move("Memory"), "Could not read image", this);
    
    if (colorType == FIC_MINISBLACK || colorType == FIC_PALETTE) {
        colorType = FIC_RGB;
        dib = FreeImage_ConvertTo24Bits(dib);
//        dib = FreeImage_ConvertToRGBF(dib);
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
        throw TextureLoadException(std::move("Memory"), "Could not read image", this);
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

    return std::make_unique<Texture>(data, size, format, static_cast<int>(format), type);
}

std::vector<std::string> TextureReaderFreeImage::supportedExtensions() const {
	// @TODO detect the supporteded extensions loaded by modules ---abock
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
