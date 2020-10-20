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

#ifndef __GHOUL___MODELMESH___H__
#define __GHOUL___MODELMESH___H__

#include <ghoul/opengl/ghoul_gl.h>

#include <vector>

namespace ghoul::opengl { class ProgramObject; }

namespace ghoul::io {

class ModelMesh {
public:
    struct Vertex {
        GLfloat location[4];
        GLfloat tex[2];
        GLfloat normal[3];
    };

    ModelMesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices);

    bool initialize(float& maximumDistanceSquared);
    void deinitialize();
    void render(ghoul::opengl::ProgramObject& program);

    void changeRenderMode(const GLenum mode);

    std::vector<Vertex> _vertices;
    std::vector<unsigned int> _indices;

private:
    GLuint _vaoID = 0;
    GLuint _vbo = 0;
    GLuint _ibo = 0;
    GLenum _mode = GL_TRIANGLES;
};

} // namespace ghoul::io

#endif // __GHOUL___MODELMESH___H__
