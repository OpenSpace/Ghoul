/*****************************************************************************************
*                                                                                       *
* OpenSpace                                                                             *
*                                                                                       *
* Copyright (c) 2014                                                                    *
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

// ghoul
#include <ghoul/io/model/modelreaderwavefront.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/opengl/vertexbufferobject.h>
#include <ghoul/glm.h>

// std
#include <vector>
#include <cstdio>
#include <iostream>
#include <fstream>

namespace {
    const std::string _loggerCat = "WavefrontGeometry";
}


namespace ghoul {
namespace io {
    
ModelReaderWavefront::ModelReaderWavefront() {
    
}

ModelReaderWavefront::~ModelReaderWavefront() {
    
}

opengl::VertexBufferObject*
ModelReaderWavefront::loadModel(const std::string& filename) const{
    
    std::ifstream file(filename, std::ios::in);
    if(!file.good())
        return nullptr;

    struct Vertex {
        GLfloat position[3];
        GLfloat tex[2];
        GLfloat normal[3];
    };
    
    // Final vertex and index vectors
    std::vector<Vertex> vertices;
    std::vector<int> indices;
    
    // temporary vectors
    std::vector<GLfloat> positions;
    std::vector<GLfloat> texcoords;
    std::vector<GLfloat> normals;
    std::vector<int> positionsIndices;
    std::vector<int> texcoordsIndices;
    std::vector<int> normalsIndices;
    
    // temporary variables for fetching
    float f1, f2, f3;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9;
    std::string line;
    
    while(std::getline(file, line)) {
        if (sscanf(line.c_str(), "v %f%f%f", &f1, &f2, &f3)) {
            positions.push_back(f1);
            positions.push_back(f2);
            positions.push_back(f3);
            continue;
        }
        if (sscanf(line.c_str(), "vn %f%f%f", &f1, &f2, &f3)) {
            normals.push_back(f1);
            normals.push_back(f2);
            normals.push_back(f3);
            continue;
        }
        if (sscanf(line.c_str(), "vt %f%f%f", &f1, &f2, &f3)) {
            texcoords.push_back(f1);
            texcoords.push_back(f2);
            texcoords.push_back(f3);
            continue;
        }
        if (texcoords.size() > 0){
            if (sscanf(line.c_str(), "f %i/%i/%i %i/%i/%i %i/%i/%i",
                       &i1, &i2, &i3, &i4, &i5, &i6, &i7, &i8, &i9))
            {
                // v1
                positionsIndices.push_back(i1-1);
                texcoordsIndices.push_back(i2-1);
                normalsIndices.push_back(i3-1);
                
                // v2
                positionsIndices.push_back(i4-1);
                texcoordsIndices.push_back(i5-1);
                normalsIndices.push_back(i6-1);
                // v3
                
                positionsIndices.push_back(i7-1);
                texcoordsIndices.push_back(i8-1);
                normalsIndices.push_back(i9-1);
                continue;
            }
        }
        else{
            if (sscanf(line.c_str(), "f %i//%i %i//%i %i//%i",
                       &i1, &i2, &i3, &i4, &i5, &i6))
            {
                // v1
                positionsIndices.push_back(i1-1);
                normalsIndices.push_back(i2-1);
                
                // v2
                positionsIndices.push_back(i3-1);
                normalsIndices.push_back(i4-1);
                
                // v3
                positionsIndices.push_back(i5-1);
                normalsIndices.push_back(i6-1);
                continue;
            }
        }
    }
    file.close();
    
    // the total number of needed
    const size_t vertexSize = positionsIndices.size() / 3;
    vertices.reserve(vertexSize);
    indices.reserve(vertexSize);
    for(size_t i = 0; i < vertexSize; ++i) {
        Vertex v;
        int normalIndex = normalsIndices.at(i) * 3;
        int textureIndex = texcoordsIndices.at(i) * 3;
        int vertexIndex = positionsIndices.at(i)*3;
        
        for (size_t j = 0; j < 3; ++j) {
            v.position[j] = positions.at(vertexIndex + j);
            v.normal[j] = positions.at(normalIndex + j);
        }
        
        // texture coordinates
        v.tex[0] = 0.0f;
        v.tex[1] = 0.0f;
        if(!texcoordsIndices.empty()) {
            v.tex[0] = texcoords.at(textureIndex);
            v.tex[1] = texcoords.at(textureIndex+1);
        }
        
        vertices.push_back(v);
        indices.push_back(i);
    }
    
    opengl::VertexBufferObject* vbo = new opengl::VertexBufferObject();
    vbo->initialize(vertices, indices);
    vbo->vertexAttribPointer(0, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, position));
    vbo->vertexAttribPointer(1, 2, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, tex));
    vbo->vertexAttribPointer(2, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, normal));
    return vbo;
}
    
} // namespace io
} // namespace ghoul

