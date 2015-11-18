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

#include <ghoul/io/texture/texturereadercmap.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/opengl/texture.h>

#include <fstream>
#include <sstream>
#include <stdint.h>

namespace {
	const std::string _loggerCat = "TextureReaderCMAP";
}

namespace ghoul {
namespace io {
namespace impl {

opengl::Texture* TextureReaderCMAP::loadTexture(const std::string& filename) const {
	std::ifstream file(filename);
	if (!file.good()) {
		LERROR("Could not open file '" << filename << "' for loading");
		return nullptr;
	}

	int width = -1; // Width of the texture
	uint8_t* values = nullptr;

	std::string line;
	int i = 0;
	while (file.good()) {
		std::getline(file, line);
		// Skip empty lines
		if (line.empty() || line == "\r")
			continue;
		// # defines a comment
		if (line[0] == '#')
			continue;

		std::stringstream s(line);
		if (width == -1) {
			// Width hasn't been set yet and we have a value, so it has to be the width
			s >> width;
			values = new uint8_t[width * 4];
			continue;
		}

		if (!values) {
			LERROR("The first non-comment, non-empty line must contain the width of "
				"the image");
			return nullptr;
		}

		float r,g,b,a;

		s >> r;
		s >> g;
		s >> b;
		s >> a;

		values[i++] = static_cast<uint8_t>(r * 255);
		values[i++] = static_cast<uint8_t>(g * 255);
		values[i++] = static_cast<uint8_t>(b * 255);
		values[i++] = static_cast<uint8_t>(a * 255);
	}

	if ((width * 4) != i) {
		LERROR("Header assured " << width << " values, but " << i / 4.f <<
			"were found");
        delete[] values;
		return nullptr;
	}

	using opengl::Texture;
	Texture* texture = new Texture(
		values,							// data
		glm::size3_t(width, 1, 1),		// dimensions
		Texture::Format::RGBA			// Format
	);

	return texture;
}

std::set<std::string> TextureReaderCMAP::supportedExtensions() const {
	return { "cmap" };
}

} // namespace impl
} // namespace io
} // namespace ghoul
