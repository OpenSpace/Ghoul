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

#include <ghoul/io/texture/texturewriter.h>

#include <ghoul/filesystem/file.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/io/texture/texturewriterbase.h>
#include <set>

namespace {
    const std::string _loggerCat = "TextureWriter";
}

namespace ghoul {
namespace io {

    TextureWriter::TextureWriter() {
        
    }
    
    TextureWriter::~TextureWriter() {
        for (TextureWriterBase* writer : _writers) {
            delete writer;
        }
    }
    
    TextureWriter& TextureWriter::ref() {
        static TextureWriter textureWriter;
        return textureWriter;
    }
    
    void TextureWriter::saveTexture(const opengl::Texture* texture, const std::string& filename) {
        if (_writers.empty()) {
            LERROR("No writers were registered with the TextureWriter");
            return;
        }
        
        std::string extension = ghoul::filesystem::File(filename).fileExtension();
        
        TextureWriterBase* writer = writerForExtension(extension);
        if (writer)
            writer->saveTexture(texture, filename);
        else {
            LERROR("No writer was found for extension '" << extension << "'");
        }
    }
    
    void TextureWriter::addWriter(TextureWriterBase* writer) {
        _writers.push_back(writer);
    }
    
    TextureWriterBase* TextureWriter::writerForExtension(const std::string& extension) {
        for (TextureWriterBase* writer : _writers) {
            std::set<std::string> extensions = writer->supportedExtensions();
            auto it = extensions.find(extension);
            if (it != extensions.end())
                return writer;
        }
            return nullptr;
    }
} // namespace io
} // namespace ghoul
