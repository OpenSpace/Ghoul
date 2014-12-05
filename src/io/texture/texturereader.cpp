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

#include <ghoul/io/texture/texturereader.h>

#include <ghoul/filesystem/file.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/io/texture/texturereaderbase.h>
#include <set>

namespace {
	const std::string _loggerCat = "TextureReader";
}

namespace ghoul {
namespace io {

std::vector<TextureReaderBase*> TextureReader::_readers;

opengl::Texture* TextureReader::loadTexture(const std::string& filename) {
	if (_readers.size() == 0) {
		LERROR("No readers were registered with the TextureReader");
		return nullptr;
	}

	std::string extension = ghoul::filesystem::File(filename).fileExtension();

	TextureReaderBase* reader = readerForExtension(extension);
	if (reader)
		return reader->loadTexture(filename);
	else
		return nullptr;
}

void TextureReader::addReader(TextureReaderBase* reader) {
	_readers.push_back(reader);
}

TextureReaderBase* TextureReader::readerForExtension(const std::string& extension) {
	for (TextureReaderBase* reader : _readers) {
		std::set<std::string> extensions = reader->supportedExtensions();
		auto it = extensions.find(extension);
		if (it != extensions.end())
			return reader;
	}
	return nullptr;
}

} // namespace io
} // namespace ghoul
