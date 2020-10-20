/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2020                                                               *
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

#include <ghoul/io/model/modelreader.h>

#include <ghoul/io/model/modelreaderbase.h>
#include <ghoul/io/model/modelgeometry.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/misc/assert.h>
#include <ghoul/fmt.h>
#include <algorithm>

namespace ghoul::io {

ModelReader::MissingReaderException::MissingReaderException(std::string extension,
                                                            std::string f)
    : RuntimeError(fmt::format("No reader was found for extension '{}' with file '{}'",
        extension,
        f
    ), "IO")
    , fileExtension(std::move(extension))
    , file(std::move(f))
{}

ModelReader& ModelReader::ref() {
    static ModelReader modelReader;
    return modelReader;
}

std::unique_ptr<modelgeometry::ModelGeometry> ModelReader::loadModel(
                                                             const std::string& filename) 
{
    ghoul_assert(!_readers.empty(), "No readers were registered before");
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    const std::string& extension = ghoul::filesystem::File(filename).fileExtension();
    ghoul_assert(!extension.empty(), "Filename must have an extension");

    ModelReaderBase* reader = readerForExtension(extension);
    if (reader) {
        return reader->loadModel(filename);
    }
    else {
        throw MissingReaderException(extension, filename);
    }
}

std::vector<std::string> ModelReader::supportedExtensions() {
    std::vector<std::string> result;
    for (const std::unique_ptr<ModelReaderBase>& i : _readers) {
        std::vector<std::string> extensions = i->supportedExtensions();
        result.insert(result.end(), extensions.begin(), extensions.end());
    }
    return result;
}

void ModelReader::addReader(std::unique_ptr<ModelReaderBase> reader) {
    _readers.push_back(std::move(reader));
}

ModelReaderBase* ModelReader::readerForExtension(const std::string& extension) {
    std::string lowerExtension = extension;
    std::transform(
        extension.cbegin(),
        extension.cend(),
        lowerExtension.begin(),
        [](char v) { return static_cast<char>(tolower(v)); }
    );
    for (const std::unique_ptr<ModelReaderBase>& reader : _readers) {
        std::vector<std::string> extensions = reader->supportedExtensions();
        auto it = std::find(extensions.cbegin(), extensions.cend(), lowerExtension);
        if (it != extensions.end()) {
            return reader.get();
        }
    }

    return nullptr;
}

} // namespace ghoul::io
