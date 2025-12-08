/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2025                                                               *
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

#include <ghoul/format.h>
#include <ghoul/io/texture/texturereaderbase.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/stringhelper.h>
#include <ghoul/opengl/texture.h>
#include <algorithm>
#include <filesystem>

namespace ghoul::io {

TextureReader::MissingReaderException::MissingReaderException(std::string extension,
                                                              std::filesystem::path file_)
    : RuntimeError(
        std::format(
            "No reader found for extension '{}' with file '{}'", extension, file_
        ),
        "IO"
    )
    , fileExtension(std::move(extension))
    , file(std::move(file_))
{}

TextureReader::InvalidLoadException::InvalidLoadException(void* memory, size_t size)
    : RuntimeError(
        std::format("Error loading texture at location {} with size {}", memory, size),
        "IO"
    )
    , _memory(memory)
    , _size(size)
{}

TextureReader& TextureReader::ref() {
    static TextureReader textureReader;
    return textureReader;
}

std::unique_ptr<opengl::Texture> TextureReader::loadTexture(
                                                    const std::filesystem::path& filename,
                                                                          int nDimensions)
{
    ghoul_assert(!_readers.empty(), "No readers were registered before");
    ghoul_assert(!filename.empty(), "Filename must not be empty");
    ghoul_assert(nDimensions >= 1 && nDimensions <= 4, "nDimensions must be 1, 2, or 3");

    std::string extension = std::filesystem::path(filename).extension().string();
    if (!extension.empty()) {
        extension = extension.substr(1);
    }
    ghoul_assert(!extension.empty(), "Filename must have an extension");

    TextureReaderBase* reader = readerForExtension(extension);
    if (!reader) {
        throw MissingReaderException(extension, filename);
    }

    return reader->loadTexture(filename, nDimensions);
}

std::unique_ptr<opengl::Texture> TextureReader::loadTexture(void* memory, size_t size,
                                                            int nDimensions,
                                                            const std::string& format)
{
    ghoul_assert(memory, "Memory must not be nullptr");
    ghoul_assert(size > 0, "Size must be > 0");
    ghoul_assert(!_readers.empty(), "No readers were registered before");

    TextureReaderBase* reader = readerForExtension(format);
    if (!reader) {
        throw InvalidLoadException(memory, size);
    }

    return reader->loadTexture(memory, size, nDimensions);
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
    const std::string lowerExtension = toLowerCase(extension);

    for (const std::unique_ptr<TextureReaderBase>& reader : _readers) {
        std::vector<std::string> extensions = reader->supportedExtensions();
        auto it = std::find(extensions.cbegin(), extensions.cend(), lowerExtension);
        if (it != extensions.end()) {
            return reader.get();
        }
    }

    return nullptr;
}

} // namespace ghoul::io
