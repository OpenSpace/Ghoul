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

#include <ghoul/opengl/vertexbufferobject.h>

namespace ghoul::opengl {

VertexBufferObject::VertexBufferObject(VertexBufferObject&& other) {
    _vaoID = other._vaoID;
    _vBufferID = other._vBufferID;
    _iBufferID = other._iBufferID;
    _iSize = other._iSize;

    other._vaoID = 0;
    other._vBufferID = 0;
    other._iBufferID = 0;
    other._iSize = 0;
}

VertexBufferObject& VertexBufferObject::operator=(VertexBufferObject&& other) {
    if (this != &other) {
        _vaoID = other._vaoID;
        _vBufferID = other._vBufferID;
        _iBufferID = other._iBufferID;
        _iSize = other._iSize;

        other._vaoID = 0;
        other._vBufferID = 0;
        other._iBufferID = 0;
        other._iSize = 0;
    }
    return *this;
}

VertexBufferObject::~VertexBufferObject() {
    glDeleteBuffers(1, &_vBufferID);
    _vBufferID = 0;

    glDeleteBuffers(1, &_iBufferID);
    _iBufferID = 0;

    glDeleteVertexArrays(1, &_vaoID);
    _vaoID = 0;

    _iSize = 0;
}

bool VertexBufferObject::isInitialized() const {
    bool initialized = true;
    initialized &= (_vaoID != 0);
    initialized &= (_vBufferID != 0);
    initialized &= (_iBufferID != 0);
    initialized &= (_iSize > 0);
    return initialized;
}

void VertexBufferObject::initialize(const std::vector<GLfloat>& vertexArray,
                                    const std::vector<GLint>& indexArray)
{
    ghoul_assert(!isInitialized(), "VertexBufferObject must not have been initialized");
    ghoul_assert(!vertexArray.empty(), "Vertex array must not be empty");
    ghoul_assert(!indexArray.empty(), "Index array must not be empty");

    generateGLObjects();

    _iSize = static_cast<unsigned int>(indexArray.size());

    glBindVertexArray(_vaoID);

    glBindBuffer(GL_ARRAY_BUFFER, _vBufferID);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertexArray.size() * sizeof(GLfloat),
        vertexArray.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iBufferID);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indexArray.size() * sizeof(GLint),
        indexArray.data(),
        GL_STATIC_DRAW
    );

    glBindVertexArray(0);
}

void VertexBufferObject::setRenderMode(GLenum mode) {
    _mode = mode;
}

void VertexBufferObject::vertexAttribPointer(GLuint index, GLint size, GLenum type,
                                             GLsizei stride, GLuint offset,
                                             GLboolean normalized)
{
    uint64_t o = offset;
    glBindVertexArray(_vaoID);
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(
        index,
        size,
        type,
        normalized,
        stride,
        reinterpret_cast<const GLvoid*>(o)
    );
    glBindVertexArray(0);
}

void VertexBufferObject::bind() {
    glBindVertexArray(_vaoID);
}

void VertexBufferObject::unbind() {
    glBindVertexArray(0);
}

void VertexBufferObject::render() {
    glBindVertexArray(_vaoID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iBufferID);
    glDrawElements(_mode, _iSize, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void VertexBufferObject::generateGLObjects() {
    glGenVertexArrays(1, &_vaoID);
    glGenBuffers(1, &_vBufferID);
    glGenBuffers(1, &_iBufferID);
}

} // namespace ghoul::opengl
