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
#include <ghoul/io/model/modelreaderlua.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/opengl/vertexbufferobject.h>
#include <ghoul/lua/lua_helper.h>
#include <ghoul/misc/dictionary.h>

// std
#include <vector>

namespace {
    const std::string _loggerCat = "ModelReaderLua";
    
    const std::string keyMode = "Mode";
    const std::string keyVertices = "Vertices";
    const std::string keyIndices = "Indices";
    const std::string keyAttribPointers = "VertexAttribPointers";
    const std::string keyPosition = "Position";
    const std::string keySize = "Size";
    const std::string keyType = "Type";
    const std::string keyStride = "Stride";
    const std::string keyOffset = "Offset";
    const std::string keyNormalized = "Normalized";
}

namespace ghoul {
namespace io {
    
ModelReaderLua::ModelReaderLua() {}

ModelReaderLua::~ModelReaderLua() {}

opengl::VertexBufferObject*
ModelReaderLua::loadModel(const std::string& filename) const{

    ghoul::Dictionary dictionary;
    bool loadSuccess = ghoul::lua::loadDictionaryFromFile(filename, dictionary);
    if(!loadSuccess) {
        LERROR("Could not load dictionary from file '" << filename << "'!");
        return nullptr;
    }
    
    bool hasKeys = true;
    hasKeys &= dictionary.hasKey(keyVertices);
    hasKeys &= dictionary.hasKey(keyIndices);
    hasKeys &= dictionary.hasKey(keyAttribPointers);
    if(!hasKeys) {
        LERROR("Missing key. Need keys, '"
               << keyVertices << "', '"
               << keyIndices << "', '"
               << keyAttribPointers << "'");
        return nullptr;
    }
    
    Dictionary vertices;
    Dictionary indices;
    Dictionary attribpointers;
    if(!dictionary.getValue(keyVertices, vertices)) {
        LERROR("Could not get vertices!");
        return nullptr;
    }
    if(!dictionary.getValue(keyIndices, indices)) {
        LERROR("Could not get indices!");
        return nullptr;
    }
    if(!dictionary.getValue(keyAttribPointers, attribpointers)) {
        LERROR("Could not get attribpointers!");
        return nullptr;
    }
    
    // Arrays
    std::vector<GLfloat> varray;
    std::vector<GLint> iarray;
    
    // get vertices
    auto vkeys = vertices.keys();
    std::sort(vkeys.begin(), vkeys.end());
    for(const auto& key: vkeys) {
        double d;
        if(vertices.getValue(key, d)) {
            varray.push_back(static_cast<GLfloat>(d));
        }
    }
    
    // get indices
    auto ikeys = vertices.keys();
    std::sort(ikeys.begin(), ikeys.end());
    for(const auto& key: ikeys) {
        double d;
        if(indices.getValue(key, d)) {
            iarray.push_back(static_cast<GLint>(d));
        }
    }
    
    // sanity check
    if(varray.empty()) {
        LERROR("No vertices specified!");
        return nullptr;
    }
    if(iarray.empty()) {
        LERROR("No indices specified!");
        return nullptr;
    }

    // everything is ok so far, create the object
    opengl::VertexBufferObject* vbo = new opengl::VertexBufferObject();
    vbo->initialize(varray, iarray);
    
    auto attribkeys = attribpointers.keys();
    for (const auto& key: attribkeys) {
        ghoul::Dictionary d;
        if(attribpointers.getValue(key, d)) {
            double tmp;
        
            int position = 0;
            int size = 0;
            GLenum type = GL_FLOAT;
            int stride = 0;
            int offset = 0;
            bool normalized = false;
            
            if(attribpointers.getValue(keyPosition, tmp))
                position = static_cast<int>(tmp);
            if(attribpointers.getValue(keySize, tmp))
                size = static_cast<int>(tmp);
            if(attribpointers.getValue(keyStride, tmp))
                stride = static_cast<int>(tmp);
            if(attribpointers.getValue(keyOffset, tmp))
                offset = static_cast<int>(tmp);
            attribpointers.getValue(keyNormalized, normalized);
            
            // @TODO: Fix some good way of supporting other types ---jonasstrandstedt
            std::string stringtype;
            if(attribpointers.getValue(keyType, stringtype)) {
                if(stringtype == "GL_FLOAT")
                    type = GL_FLOAT;
                //else if(stringtype == "GL_INT")
                //    type = GL_INT;
            }
            
            vbo->vertexAttribPointer(position, size, type, stride, offset, normalized);
        }
    }
    
    // get and set render mode
    std::string mode;
    if(dictionary.getValue(keyMode, mode)) {
        if(mode == "GL_POINTS")
            vbo->setRenderMode(GL_POINTS);
        else if(mode == "GL_LINE_STRIP")
            vbo->setRenderMode(GL_LINE_STRIP);
        else if(mode == "GL_LINE_LOOP")
            vbo->setRenderMode(GL_LINE_LOOP);
        else if(mode == "GL_LINES")
            vbo->setRenderMode(GL_LINES);
        else if(mode == "GL_LINE_STRIP_ADJACENCY")
            vbo->setRenderMode(GL_LINE_STRIP_ADJACENCY);
        else if(mode == "GL_LINES_ADJACENCY")
            vbo->setRenderMode(GL_LINES_ADJACENCY);
        else if(mode == "GL_TRIANGLE_STRIP")
            vbo->setRenderMode(GL_TRIANGLE_STRIP);
        else if(mode == "GL_TRIANGLE_FAN")
            vbo->setRenderMode(GL_TRIANGLE_FAN);
        else if(mode == "GL_TRIANGLES")
            vbo->setRenderMode(GL_TRIANGLES);
        else if(mode == "GL_TRIANGLE_STRIP_ADJACENCY")
            vbo->setRenderMode(GL_TRIANGLE_STRIP_ADJACENCY);
        else if(mode == "GL_TRIANGLES_ADJACENCY")
            vbo->setRenderMode(GL_TRIANGLES_ADJACENCY);
        else if(mode == "GL_PATCHES")
            vbo->setRenderMode(GL_PATCHES);
    }
    
    return vbo;
}
    
} // namespace io
} // namespace ghoul

