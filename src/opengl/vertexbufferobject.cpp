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

#include <ghoul/opengl/vertexbufferobject.h>

namespace ghoul {
namespace opengl {

VertexBufferObject::VertexBufferObject()
	: _vaoID(0)
	, _vBufferID(0)
	, _iBufferID(0)
	, _isize(0)
{}

VertexBufferObject::~VertexBufferObject() {
	glDeleteBuffers(1, &_vBufferID);
	glDeleteBuffers(1, &_iBufferID);
	glDeleteVertexArrays(1, &_vaoID);
}

void VertexBufferObject::vertexAttribPointer(
	GLuint index, 
	GLint size,
	GLenum type,
	GLsizei stride, 
	GLuint offset,  
	GLboolean normalized)
{
	glBindVertexArray(_vaoID);
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type, normalized, stride,
		reinterpret_cast<const GLvoid*>(offset));
	glBindVertexArray(0);
}

void VertexBufferObject::bind() {
	glBindVertexArray(_vaoID);
}

void VertexBufferObject::unbind() {
	glBindVertexArray(0);
}

void VertexBufferObject::render(GLenum mode) {
	glBindVertexArray(_vaoID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iBufferID);
	glDrawElements(mode, _isize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

}
}