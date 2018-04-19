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

#include <ghoul/io/model/modelreaderwavefront.h>

#include <ghoul/fmt.h>
#include <ghoul/misc/assert.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/opengl/vertexbufferobject.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <tiny_obj_loader.h>
#include <vector>

namespace ghoul::io {

std::unique_ptr<opengl::VertexBufferObject> ModelReaderWavefront::loadModel(
                                                        const std::string& filename) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string errorMessage;
    bool success = tinyobj::LoadObj(
        shapes,
        materials,
        errorMessage,
        filename.c_str(),
        filename.c_str()
    );

    if (!success) {
        throw ModelReaderException(filename, errorMessage);
    }

    size_t totalSizeIndex = 0;
    size_t totalSizeVertex = 0;
    for (size_t i = 0; i < shapes.size(); ++i) {
        totalSizeIndex += shapes[i].mesh.indices.size();
        totalSizeVertex += shapes[i].mesh.positions.size();

        if (shapes[i].mesh.positions.size() != shapes[i].mesh.normals.size()) {
            throw ModelReaderException(
                filename,
                fmt::format(
                    "Malformed OBJ file: Number of positions {} != number of normals",
                    shapes[i].mesh.positions.size(),
                    shapes[i].mesh.normals.size()
                )
            );
        }
    }

    /*struct Vertex {
        GLfloat location[3];
        GLfloat tex[2];
        GLfloat normal[3];
    };*/
    std::vector<Vertex> vertices(totalSizeVertex);
    std::vector<int> indices(totalSizeIndex);

    // We add all shapes of the model into the same vertex array, one after the other
    // The _shapeCounts array stores for each shape, how many vertices that shape has
    size_t positionIndex = 0;
    size_t indicesIndex = 0;
    for (size_t i = 0; i < shapes.size(); ++i) {
        for (size_t j = 0; j < shapes[i].mesh.positions.size() / 3; ++j) {
            vertices[j + positionIndex].location[0] = shapes[i].mesh.positions[3 * j + 0];
            vertices[j + positionIndex].location[1] = shapes[i].mesh.positions[3 * j + 1];
            vertices[j + positionIndex].location[2] = shapes[i].mesh.positions[3 * j + 2];

            vertices[j + positionIndex].normal[0] = shapes[i].mesh.normals[3 * j + 0];
            vertices[j + positionIndex].normal[1] = shapes[i].mesh.normals[3 * j + 1];
            vertices[j + positionIndex].normal[2] = shapes[i].mesh.normals[3 * j + 2];

            if (2 * j + 1 < shapes[i].mesh.texcoords.size()) {
                vertices[j + positionIndex].tex[0] = shapes[i].mesh.texcoords[2 * j + 0];
                vertices[j + positionIndex].tex[1] = shapes[i].mesh.texcoords[2 * j + 1];
            }
        }
        positionIndex += shapes[i].mesh.positions.size() / 3;

        std::copy(
            shapes[i].mesh.indices.begin(),
            shapes[i].mesh.indices.end(),
            indices.begin() + indicesIndex
        );
        indicesIndex += shapes[i].mesh.indices.size();
    }

    std::unique_ptr<opengl::VertexBufferObject> vbo =
        std::make_unique<opengl::VertexBufferObject>();

    vbo->initialize(vertices, indices);
    vbo->vertexAttribPointer(0, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, location));
    vbo->vertexAttribPointer(1, 2, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, tex));
    vbo->vertexAttribPointer(2, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, normal));
    return vbo;
}

} // namespace ghoul::io

