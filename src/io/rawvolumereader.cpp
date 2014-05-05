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

#include <ghoul/io/rawvolumereader.h>
#include <iostream>

namespace ghoul {

RawVolumeReader::ReadHints::ReadHints(glm::ivec3 dimensions) : _dimensions(dimensions) {
}

RawVolumeReader::RawVolumeReader() {
	_hints = ReadHints();
}

RawVolumeReader::RawVolumeReader(const ReadHints& hints) {
	_hints = hints;
}

RawVolumeReader::~RawVolumeReader() {
}

void RawVolumeReader::setReadHints(glm::ivec3 dimension) {
	_hints._dimensions = dimension;
}

void RawVolumeReader::setReadHints(const ReadHints& hints) {
	_hints = hints;
}

opengl::Texture* RawVolumeReader::read(std::string filename) {
	if (_hints._dimensions != glm::ivec3(0)) {
		int size = _hints._dimensions.x*_hints._dimensions.y*_hints._dimensions.z;
		GLubyte *data = new GLubyte[size];

		if( FILE *fin = fopen(filename.c_str(), "rb") ){
			fread(data, sizeof(unsigned char), size, fin);
			fclose(fin);
		} else {
			fprintf( stderr, "Could not open file '%s'\n", filename.c_str() );
		}

		opengl::Texture* texture = new opengl::Texture(data, glm::size3_t(_hints._dimensions),
				_hints._format, _hints._internalFormat, GL_UNSIGNED_BYTE,
				opengl::Texture::FilterMode::Linear, opengl::Texture::WrappingMode::ClampToBorder);
		//texture->uploadTexture();
		return texture;
	} else {
		std::cout << "Error. Volume dimensions not set" << std::endl;
		return nullptr;
	}
}

} // namespace ghoul
