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

// ghoul
#include <ghoul/opengl/ghoul_gl.h>

// std
#include <vector>

namespace ghoul {
namespace opengl {

/**
 * This class is a wrapper for handling vertex buffer objects. It is only meant for 
 * simplifying the creation and use of the most standard vertex buffer object with a 
 * vertex list and an index list. 
 */
class VertexBufferObject {
public:

    /**
     * Default constructor. Initializes the internal GL objects to 0.
     * VertexBufferObjects can be constructed withouta GL context but
     * cannot be initialized without a GL context.
     */
    VertexBufferObject();
    
    /**
     * Moves the other object in place of the created one. The other object
     * will be in an uninitialized state afterwards.
     */
    VertexBufferObject(VertexBufferObject&& other);
    
    /**
     * Moves the other object in place of the created one. The other object
     * will be in an uninitialized state afterwards.
     */
    VertexBufferObject& operator=(VertexBufferObject&& other);

	/**
	 * Default destructor. Calls an assertion in debug mode that the VertexBufferObject 
	 * is not still initialized.
	 */
	~VertexBufferObject();
    
    /**
     * A runtime function that checks if initialize has been called.
     * \returns <code>true</code> if any initialize function has been 
     * called and <code>false</code> otherwise.
     */
    bool isInitialized() const;
    
    /**
     * Initializes a VBO with a vertex and index list. The structure of the vertex data
     * can be anything. Make sure to specify the correct offsets with vertexAttribPointer.
     * \param varray The vertex array
     * \param iarray The index list
     */
    bool initialize(const std::vector<GLfloat>& varray, const std::vector<GLint>& iarray);

	/**
	 * Initializes the VBO with the vertex and index list
	 * \param varray The vertex array
	 * \param iarray The index list
	 */
	template<typename T>
	bool initialize(const std::vector<T>& varray, const std::vector<GLint>& iarray);

	/**
	 * Deinitializes the internal GL objects, if the VertexBufferObject is 
	 * initialized then deinitialize must be done before destruction of 
	 * the object.
	 */
	void deinitialize();
    
    /**
     * Sets the render mode for this VertexBufferObject. The render mode is how OpenGL is 
     * treating the vertices and indices. The default is <code>GL_TRIANGLES</code> but
     * other common render primitives are <code>GL_LINES</code> and
     * <code>GL_POINTS</code>. See https://www.opengl.org/sdk/docs/man/html/glDrawElements.xhtml
     * for supported render modes.
     * \param mode The render mode. Default is <code>GL_TRIANGLES</code>
     */
    void setRenderMode(GLenum mode = GL_TRIANGLES);

	/**
	 * A wrapper function for glEnableVertexAttribArray 
	 * and glVertexAttribPointer
	 * \param index The index of the attribute
	 * \param size The number of elements 
	 * \param type The data type, for example GL_FLOAT
	 * \param stride The size of the Vertex struct
	 * \param offset The offset for the specific member
	 * \param normalized flag weather the attribute should be normalized.
     * Defaults is <code>GL_FALSE</code>
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
	 * Renders the VBO using the provided mode.
	 * \param mode The render mode, default is <code>GL_TRIANGLES</code>
	 */
	void render();

private:
    
    // no need and no point in copying VertexBufferObjects
    VertexBufferObject(const VertexBufferObject&) = delete;
    VertexBufferObject& operator=(const VertexBufferObject&) = delete;

	/**
	 * Constructs the internal GL objects by calling 
	 * glGenVertexArrays and glGenBuffers
	 */
	void generateGLObjects();

	GLuint _vaoID;
	GLuint _vBufferID;
    GLuint _iBufferID;
    unsigned int _isize;
    GLenum _mode;

}; // class VertexBufferObject

} // namespace opengl
} // namespace ghoul

#include "vertexbufferobject.inl"

#endif // __VERTEXBUFFEROBJECT_H__