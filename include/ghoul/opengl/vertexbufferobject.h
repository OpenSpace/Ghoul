/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2016                                                               *
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

/**
 * This class is a wrapper for handling vertex buffer objects. It is only meant for 
 * simplifying the creation and use of the most standard vertex buffer object with a 
 * vertex list and an index list. It will create an internal vertex array object to store
 * the created VertexBufferObject.
 */
class VertexBufferObject {
public:

    /**
     * Default constructor. Initializes the internal GL objects to 0. VertexBufferObject%s
     * cannot be constructed without an OpenGL context.
     */
    VertexBufferObject();
    
    /**
     * Moves the other object in place of the created one. The other object will be in an
     * uninitialized state afterwards.
     */
    VertexBufferObject(VertexBufferObject&& other);
    
    /**
     * Moves the other object in place of the created one. The other object will be in an
     * uninitialized state afterwards.
     */
    VertexBufferObject& operator=(VertexBufferObject&& other);

    /**
     * Default destructor.
     */
    ~VertexBufferObject();
    
    /**
     * A runtime function that checks if initialize has been called.
     * \returns <code>true</code> if any initialize function has been called and
     * <code>false</code> otherwise.
     */
    bool isInitialized() const;
    
    /**
     * Initializes the VertexBufferObject with the provided \p vertexArray and
     * \p indexArray list. The structure of the vertex data can be arbitrary but correct
     * offsets must be specified using #vertexAttribPointer.
     * \param vertexArray The vertex array used for this VertexBufferObject
     * \param indexArray The index list used for this VertexBufferObject
     * \pre VertexBufferObject must not have been initialized
     * \pre \p vertexArray must not be empty
     * \pre \p indexArray must not be empty
     */
    void initialize(const std::vector<GLfloat>& vertexArray,
        const std::vector<GLint>& indexArray);

    /**
     * Initializes the VertexBufferObject with the provided \p vertexArray and
     * \p indexArray list. The structure of the vertex data can be arbitrary but correct
     * offsets must be specified using #vertexAttribPointer.
     * \tparam T The structore holding information for each vertex; must be a
     * <code>POD</code>
     * \param vertexArray The vertex array used for this VertexBufferObject
     * \param indexArray The index list used for this VertexBufferObject
     * \pre VertexBufferObject must not have been initialized before
     */
    template<typename T>
    void initialize(const std::vector<T>& vertexArray,
        const std::vector<GLint>& indexArray);

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
     * A wrapper function for <code>glEnableVertexAttribArray</code> and
     * <code>glVertexAttribPointer</code> that defines how the values passed in the
     * #initialize method are interpreted.
     * \param index The index of the attribute to be modified
     * \param size The number of elements in this attribute
     * \param type The data type, for example <code>GL_FLOAT</code>
     * \param stride The size for all values of a vertex
     * \param offset The offset for the specific member
     * \param normalized flag weather the attribute should be normalized
     */
    void vertexAttribPointer(GLuint index, GLint size, GLenum type, GLsizei stride,
        GLuint offset, GLboolean normalized = GL_FALSE);

    /// Binds the VBO
    void bind();

    /// Unbinds the VBO
    void unbind();

    /// Render the VBO using the provided mode through VertexBufferObject::setRenderMode.
    void render();

private:
    /**
     * Constructs the internal GL objects by calling <code>glGenVertexArrays</code> and
     * <code>glGenBuffers</code>
     */
    void generateGLObjects();

    /// The vertex array object that stores the created VertexBufferObject
    GLuint _vaoID;
    
    /// The vertex buffer object
    GLuint _vBufferID;
    
    /// The index buffer obejct
    GLuint _iBufferID;
    
    /// The size of the index buffer, determining how many vertices are drawn
    unsigned int _iSize;
    
    /// The rendering mode of this VertexBufferObject
    GLenum _mode;
};

} // namespace opengl
} // namespace ghoul

#include "vertexbufferobject.inl"

#endif // __VERTEXBUFFEROBJECT_H__
