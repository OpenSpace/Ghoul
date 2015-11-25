/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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

#include <ghoul/io/texture/texturereaderdevil.h>

#ifdef GHOUL_USE_DEVIL

#include <ghoul/opengl/texture.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/glm.h>

#include <format.h>

#include <IL/il.h>
#include <IL/ilu.h>

namespace {
	const std::string _loggerCat = "TextureReaderDevIL";
}

namespace ghoul {
namespace io {

std::unique_ptr<opengl::Texture> TextureReaderDevIL::loadTexture(std::string filename) const {
	using opengl::Texture;
    ilInit();
    iluInit();
    
    //ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    //ilEnable(IL_ORIGIN_SET);

    ILboolean loadSuccess = ilLoadImage(filename.c_str());
    if (!loadSuccess) {
        ILenum error = ilGetError();
		throw TextureLoadException(
			std::move(filename),
			fmt::format("Error loading image: {}", iluErrorString(error)),
			this
		);
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
			throw TextureLoadException(
				std::move(filename),
				fmt::format("Error reading format: {}", imageFormat),
				this
			);
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
			throw TextureLoadException(
				std::move(filename),
				fmt::format("Error reading data type: {}", imageType),
				this
			);
    }

	return std::make_unique<Texture>(data, size, format, static_cast<int>(format), type);
}

std::vector<std::string> TextureReaderDevIL::supportedExtensions() const {
	// Taken from http://openil.sourceforge.net/features.php
	return {
		"bmp",						// Windows Bitmap
		"cut",						// Dr. Halo
		"dcx",						// Multi-PCX
		"dicom", "dcm",				// Dicom
		"dds",						// DirectDraw Surface
		"exr",						// OpenEXR
		"fits", "fit",				// Flexible Image Transport System
		"ftx",						// Heavy Metal: FAKK 2
		"hdr",						// Radiance High Dynamic
		"icns",						// Macintosh icon
		"ico", "cur",				// Windows icon/cursor
		"iff",						// Interchange File Format
		"iwi",						// Infinity Ward Image
		"gif",						// Graphics Interchange Format
		"jpg", "jpe", "jpeg",		// Jpeg
		"jp2",						// Jpeg 2000
		"lbm",						// Interlaced Bitmap
		"lif",						// Homeworld texture
		"mdl",						// Half-Life Model
		"mp3",						// MPEG-1 Audio Layer 3
		"pal",						// Palette
		"pcd",						// Kodak PhotoCD
		"pcx",						// ZSoft PCX
		"pic",						// Softimage PIC
		"png",						// Portable Network Graphics
		"pbm", "pgm", "pnm",		// Portable Anymap
		"pix",						// Alias | Wavefront
		"psd",						// Adobe PhotoShop
		"psp",						// PaintShop Pro
		"pxr",						// Pixar
		"raw",						// Raw data
		"rot",						// Homeworld 2 Texture
		"sgi", "bw", "rgb", "rgba"	// Silicon Graphics
		"texture",					// Creative Assembly Texture
		"tga",						// Truevision Targa
		"tif", "tiff",				// Tagged Image File Format
		"tpl",						// Gamecube Texture
		"utx",						// Unreal Texture
		"wal",						// Quake 2 Texture
		"vtf",						// Valve Texture Format
		"wdp", "hdp",				// HD Photo
		"xpm"						// X Pixel Map
	};
}

} // namespace opengl
} // namespace ghoul

#endif // GHOUL_USE_DEVIL
