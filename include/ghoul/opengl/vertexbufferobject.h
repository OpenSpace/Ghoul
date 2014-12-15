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

#ifndef __VERTEXBUFFEROBJECT_H__
#define __VERTEXBUFFEROBJECT_H__
#include <ghoul/opengl/ghoul_gl.h>

#include <vector>

namespace ghoul {
namespace opengl {


class VertexBufferObject {
public:
	VertexBufferObject();
	~VertexBufferObject();

	/**
	 * Initializes the VBO with the vertex and index list
	 * \param varray The vertex array
	 * \param iarray The index list
	 */
	template<typename T>
	bool initialize(const std::vector<T>& varray, const std::vector<GLint>& iarray);

	/**
	 * A wrapper function for glEnableVertexAttribArray 
	 * and glVertexAttribPointer
	 * \param index The index of the attribute
	 * \param size The number of elements 
	 * \param type The data type, for example GL_FLOAT
	 * \param stride The size of the Vertex struct
	 * \param offset The offset for the specific member
	 * \param normalized Flag weather the parameter should be normalized. Defaults to GL_FALSE
	 */
	void vertexAttribPointer(
		GLuint index, 
		GLint size,
		GLenum type,
		GLsizei stride, 
		GLuint offset, 
		GLboolean normalized = GL_FALSE);

	/**
	 * Binds the VBO
	 */
	void bind();

	/**
	 * Unbinds the VBO
	 */
	void unbind();

	/**
	 * Renders the VBO using the provided mode. Default is GL_TRIANGLES.
	 * \param mode The render mode
	 */
	void render(GLenum mode = GL_TRIANGLES);

private:

	GLuint _vaoID;
	GLuint _vBufferID;
	GLuint _iBufferID;
	unsigned int _isize;
}; // class VertexBufferObject



template<typename T>
bool VertexBufferObject::initialize(const std::vector<T>& varray, const std::vector<GLint>& iarray) {
	static_assert(std::is_pod<T>::value, "T has to be a POD");

	glGenVertexArrays(1, &_vaoID);
	glGenBuffers(1, &_vBufferID);
	glGenBuffers(1, &_iBufferID);

	if (_vaoID == 0 || _vBufferID == 0 || _iBufferID == 0)
		return false;

	if (varray.size() == 0 || iarray.size() == 0)
		return false;

	_isize = iarray.size();

	glBindVertexArray(_vaoID);

	glBindBuffer(GL_ARRAY_BUFFER, _vBufferID);
	glBufferData(GL_ARRAY_BUFFER, varray.size() * sizeof(T), varray.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iarray.size() * sizeof(GLint), iarray.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

/*
template<typename T>
VertexBufferObject<T>::VertexBufferObject()
	: _vaoID(0)
	, _vBufferID(0)
	, _iBufferID(0)
	, _isize(0)
{}

template<typename T>
VertexBufferObject<T>::~VertexBufferObject() {
	glDeleteBuffers(1, &_vBufferID);
	glDeleteBuffers(1, &_iBufferID);
	glDeleteVertexArrays(1, &_vaoID);
}
template<typename T>
void VertexBufferObject<T>::vertexAttribPointer(GLuint index, GLint size, GLuint offset, GLenum type, GLboolean normalized){
	glBindVertexArray(_vaoID);
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type, normalized, sizeof(T),
		reinterpret_cast<const GLvoid*>(offset));
	glBindVertexArray(0);
}

template<typename T>
void VertexBufferObject<T>::bind() {
	glBindVertexArray(_vaoID);
}

template<typename T>
void VertexBufferObject<T>::unbind() {
	glBindVertexArray(0);
}

template<typename T>
void VertexBufferObject<T>::render() {
	glBindVertexArray(_vaoID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iBufferID);
	glDrawElements(GL_TRIANGLES, _isize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
*/
} // namespace opengl
} // namespace ghoul

#endif // __VERTEXBUFFEROBJECT_H__