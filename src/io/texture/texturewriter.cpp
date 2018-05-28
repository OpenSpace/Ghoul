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

#include <ghoul/io/texture/texturewriter.h>

#include <ghoul/fmt.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/io/texture/texturewriterbase.h>
#include <ghoul/opengl/texture.h>
#include <algorithm>

using std::string;

namespace ghoul::io {

TextureWriter::MissingWriterException::MissingWriterException(std::string extension)
    : RuntimeError(fmt::format("No writer was found for extension '{}'", extension), "IO")
    , fileExtension(std::move(extension))
{}

TextureWriter& TextureWriter::ref() {
    static TextureWriter textureWriter;
    return textureWriter;
}

void TextureWriter::saveTexture(const opengl::Texture& texture, const string& filename) {
    ghoul_assert(!_writers.empty(), "No writers were registered before");
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    ghoul::filesystem::File file = ghoul::filesystem::File(filename);
    const std::string& extension = file.fileExtension();
    ghoul_assert(!extension.empty(), "Filename must have an extension");

    TextureWriterBase* writer = writerForExtension(extension);

    if (writer) {
        // Make sure the directory for the file exists
        FileSys.createDirectory(file.directoryName());
        writer->saveTexture(texture, filename);
    }
    else {
        throw MissingWriterException(extension);
    }
}

void TextureWriter::addWriter(std::unique_ptr<TextureWriterBase> writer) {
    _writers.push_back(std::move(writer));
}

TextureWriterBase* TextureWriter::writerForExtension(const std::string& extension) {
    for (const std::unique_ptr<TextureWriterBase>& writer : _writers) {
        std::vector<std::string> extensions = writer->supportedExtensions();
        auto it = std::find(extensions.begin(), extensions.end(), extension);
        if (it != extensions.end()) {
            return writer.get();
        }
    }
    return nullptr;
}

} // namespace ghoul::io
