/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2026                                                               *
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

#include <ghoul/misc/assert.h>
#include <type_traits>

namespace ghoul::opengl {

template <typename T>
void VertexBufferObject::initialize(const std::vector<T>& vertexArray,
                                    const std::vector<GLint>& indexArray)
{
    static_assert(std::is_trivially_copyable_v<T>, "T has to be trivially copyable");

    ghoul_assert(!isInitialized(), "VertexBufferObject must not have been initialized");
    ghoul_assert(!vertexArray.empty(), "Vertex array must not be empty");
    ghoul_assert(!indexArray.empty(), "Index array must not be empty");

    generateGLObjects();

    _iSize = static_cast<unsigned int>(indexArray.size());

    glBindVertexArray(_vaoID);

    glBindBuffer(GL_ARRAY_BUFFER, _vBufferID);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertexArray.size() * sizeof(T),
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

} // namespace ghoul::opengl
