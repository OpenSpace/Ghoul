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

#include <ghoul/io/model/modelreaderlua.h>

#include <ghoul/fmt.h>
#include <ghoul/lua/lua_helper.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/opengl/vertexbufferobject.h>
#include <map>
#include <vector>

using fmt::format;

namespace {
    constexpr const char* keyMode = "Mode";
    constexpr const char* keyVertices = "Vertices";
    constexpr const char* keyIndices = "Indices";
    constexpr const char* keyAttribPointers = "VertexAttribPointers";
    constexpr const char* keyPosition = "Position";
    constexpr const char* keySize = "Size";
    constexpr const char* keyStride = "Stride";
    constexpr const char* keyOffset = "Offset";
    constexpr const char* keyNormalized = "Normalized";
} // namespace

namespace ghoul::io {

std::unique_ptr<opengl::VertexBufferObject> ModelReaderLua::loadModel(
                                                        const std::string& filename) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    Dictionary dictionary;
    try {
        lua::loadDictionaryFromFile(filename, dictionary);
    }
    catch (const ghoul::lua::LuaRuntimeException& e) {
        throw ModelReaderException(filename, e.what());
    }

    if (!dictionary.hasKeyAndValue<Dictionary>(keyVertices)) {
        throw ModelReaderException(
            filename, format("Missing key or wrong format for '{}'", keyVertices)
        );
    }
    if (!dictionary.hasKeyAndValue<Dictionary>(keyIndices)) {
        throw ModelReaderException(
            filename, format("Missing key or wrong format for '{}'", keyIndices)
        );
    }
    if (!dictionary.hasKeyAndValue<Dictionary>(keyAttribPointers)) {
        throw ModelReaderException(
            filename, format("Missing key or wrong format for '{}'", keyAttribPointers)
        );
    }
    if (!dictionary.hasKeyAndValue<std::string>(keyMode)) {
        throw ModelReaderException(
            filename, format("Missing key or wrong format for '{}'", keyMode)
        );
    }

    // get vertices
    Dictionary vertices = dictionary.value<Dictionary>(keyVertices);
    std::vector<GLfloat> varray;
    for (size_t i = 1; i <= vertices.size(); ++i) {
        double d = vertices.value<double>(std::to_string(i));
        varray.push_back(static_cast<GLfloat>(d));
    }

    // get indices
    Dictionary indices = dictionary.value<Dictionary>(keyIndices);
    std::vector<GLint> iarray;
    for (size_t i = 1; i <= indices.size(); ++i) {
        double d = indices.value<double>(std::to_string(i));
        iarray.push_back(static_cast<GLint>(d));
    }

    if (varray.empty()) {
        throw ModelReaderException(filename, "No vertices specified");
    }
    if (iarray.empty()) {
        throw ModelReaderException(filename, "No indices specified");
    }

    // Create the resulting VBO
    std::unique_ptr<opengl::VertexBufferObject> vbo =
        std::make_unique<opengl::VertexBufferObject>();
    vbo->initialize(varray, iarray);

    Dictionary attribPointers = dictionary.value<Dictionary>(keyAttribPointers);
    std::vector<std::string> attribKeys = attribPointers.keys();
    for (const std::string& key : attribKeys) {
        ghoul::Dictionary d;
        if (attribPointers.getValue(key, d)) {
            double position = 0.0;
            attribPointers.getValue(keyPosition, position);

            double size = 0.0;
            attribPointers.getValue(keySize, size);

            double stride = 0.0;
            attribPointers.getValue(keyStride, stride);

            double offset = 0.0;
            attribPointers.getValue(keyOffset, offset);

            bool normalized = false;
            attribPointers.getValue(keyNormalized, normalized);

            GLenum type = GL_FLOAT;
            vbo->vertexAttribPointer(
                static_cast<GLuint>(position),
                static_cast<GLint>(size),
                type,
                static_cast<GLsizei>(stride),
                static_cast<GLuint>(offset),
                static_cast<GLboolean>(normalized)
            );
        }
    }

    static const std::map<std::string, GLenum> ModeMap = {
        {"GL_POINTS", GL_POINTS},
        {"GL_LINE_STRIP", GL_LINE_STRIP},
        {"GL_LINE_LOOP", GL_LINE_LOOP},
        {"GL_LINES", GL_LINES},
        {"GL_LINE_STRIP_ADJACENCY", GL_LINE_STRIP_ADJACENCY},
        {"GL_LINES_ADJACENCY", GL_LINES_ADJACENCY},
        {"GL_TRIANGLE_STRIP", GL_TRIANGLE_STRIP},
        {"GL_TRIANGLE_FAN", GL_TRIANGLE_FAN},
        {"GL_TRIANGLES", GL_TRIANGLES},
        {"GL_TRIANGLE_STRIP_ADJACENCY", GL_TRIANGLE_STRIP_ADJACENCY},
        {"GL_TRIANGLES_ADJACENCY", GL_TRIANGLES_ADJACENCY},
        {"GL_PATCHES", GL_PATCHES}
    };

    std::string modeString = dictionary.value<std::string>(keyMode);
    auto it = ModeMap.find(modeString);
    if (it == ModeMap.end()) {
        throw ModelReaderException(
            filename, format("Illegal rendering mode '{}'", modeString)
        );
    }

    GLenum mode = ModeMap.at(modeString);
    vbo->setRenderMode(mode);
    return vbo;
}

} // namespace ghoul::io

