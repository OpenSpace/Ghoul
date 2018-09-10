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

#include <ghoul/io/texture/texturereader.h>

#include <ghoul/fmt.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/io/texture/texturereaderbase.h>
#include <ghoul/misc/assert.h>
#include <ghoul/opengl/texture.h>
#include <algorithm>

namespace ghoul::io {

TextureReader::MissingReaderException::MissingReaderException(std::string extension,
                                                              std::string f)
    : RuntimeError(fmt::format("No reader was found for extension '{}' with file '{}'",
        extension,
        f
    ), "IO")
    , fileExtension(std::move(extension))
    , file(std::move(f))
{}

TextureReader& TextureReader::ref() {
    static TextureReader textureReader;
    return textureReader;
}

std::unique_ptr<opengl::Texture> TextureReader::loadTexture(const std::string& filename) {
    ghoul_assert(!_readers.empty(), "No readers were registered before");
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    const std::string& extension = ghoul::filesystem::File(filename).fileExtension();
    ghoul_assert(!extension.empty(), "Filename must have an extension");

    TextureReaderBase* reader = readerForExtension(extension);
    if (reader) {
        return reader->loadTexture(filename);
    }
    else {
        throw MissingReaderException(extension, filename);
    }
}

std::unique_ptr<opengl::Texture> TextureReader::loadTexture(void* memory,
                                                            size_t size,
                                                            const std::string& format)
{
    ghoul_assert(memory, "Memory must not be nullptr");
    ghoul_assert(size > 0, "Size must be > 0");
    ghoul_assert(!_readers.empty(), "No readers were registered before");

    TextureReaderBase* reader = readerForExtension(format);
    return reader->loadTexture(memory, size);
}

std::vector<std::string> TextureReader::supportedExtensions() {
    std::vector<std::string> result;
    for (const std::unique_ptr<TextureReaderBase>& i : _readers) {
        std::vector<std::string> extensions = i->supportedExtensions();
        result.insert(result.end(), extensions.begin(), extensions.end());
    }
    return result;
}

void TextureReader::addReader(std::unique_ptr<TextureReaderBase> reader) {
    _readers.push_back(std::move(reader));
}

TextureReaderBase* TextureReader::readerForExtension(const std::string& extension) {
    std::string lowerExtension = extension;
    std::transform(
        extension.begin(),
        extension.end(),
        lowerExtension.begin(),
        [](char v) { return static_cast<char>(tolower(v)); }
    );
    for (const std::unique_ptr<TextureReaderBase>& reader : _readers) {
        std::vector<std::string> extensions = reader->supportedExtensions();
        auto it = std::find(extensions.begin(), extensions.end(), lowerExtension);
        if (it != extensions.end()) {
            return reader.get();
        }
    }

    return nullptr;
}

} // namespace ghoul::io
