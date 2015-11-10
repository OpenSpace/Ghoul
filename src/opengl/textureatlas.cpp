/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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

#include "ghoul/opengl/textureatlas.h"

#include "ghoul/logging/logmanager.h"
#include "ghoul/misc/assert.h"
#include "ghoul/opengl/ghoul_gl.h"

namespace {
    const std::string _loggerCat = "TextureAtlas";
}

namespace ghoul {
namespace opengl {
    
TextureAtlas::TextureAtlas(int width, int height, int depth)
    : _width(width)
    , _height(height)
    , _depth(depth)
    , _nUsed(0)
    , _id(0)
    , _data(nullptr)
{
    ghoul_assert(width > 4, "Width has to be bigger than 4");
    ghoul_assert(height > 4, "Height has to be bigger than 4");
    ghoul_assert(depth >= 1, "Depth has to be positive");
    ghoul_assert(depth <= 4, "Depth has to be smaller or equal to 4");
    // Limitations to the depth are due to the fact that the atlas is represented by
    // a single texture on the GPU (which only allows up to four channels)
    
    _nodes.emplace_back(1, 1, width - 2);

    _data = new unsigned char[width * height * depth];
    memset(_data, 0, _width * _height * _depth);
}
    
TextureAtlas::~TextureAtlas() {
    delete[] _data;
    if (_id)
        glDeleteTextures(1, &_id);
}
    
    
int TextureAtlas::width() const {
    return _width;
}

int TextureAtlas::height() const {
    return _height;
}
    
int TextureAtlas::depth() const {
    return _depth;
}
    
unsigned int TextureAtlas::id() const {
    return _id;
}

void TextureAtlas::upload() {
    // TODO: Change to use ghoul::opengl::Texture class instead ---abock
    if (!_id)
        glGenTextures(1, &_id);
    
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    switch (_depth) {
        case 1:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _width, _height,
                         0, GL_RED, GL_UNSIGNED_BYTE, _data);
            break;
        case 2:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, _width, _height,
                         0, GL_RG, GL_UNSIGNED_BYTE, _data);
            break;
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height,
                         0, GL_RGB, GL_UNSIGNED_BYTE, _data);
            break;
        case 4:
#ifdef GL_UNSIGNED_INT_8_8_8_8_REV
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height,
                         0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, _data);
#else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height,
                         0, GL_RGBA, GL_UNSIGNED_BYTE, _data);
#endif
            break;
    }
}

void TextureAtlas::clear() {
    _nodes.clear();
    _nodes.emplace_back(1, 1 , _width - 2);

    _nUsed = 0;
    
    memset(_data, 0, _width * _height * _depth);
}
    
glm::ivec4 TextureAtlas:: allocateRegion(int width, int height) {
    glm::ivec4 region(0, 0, width, height);

    int bestHeight = std::numeric_limits<int>::max();
    int bestWidth = std::numeric_limits<int>::max();
    int bestIndex = -1;
    
    for (size_t i = 0; i < _nodes.size(); ++i) {
        int y = atlasFit(i, width, height);
        if (y >= 0) {
            const glm::ivec3& node = _nodes[i];
            if (((y + height) < bestHeight ) ||
               (((y + height) == bestHeight) && (node.z < bestWidth)))
            {
                bestHeight = y + height;
                bestIndex = i;
                bestWidth = node.z;
                region.x = node.x;
                region.y = y;
            }
        }
    }
    
    if (bestIndex == -1)
        return glm::ivec4(-1, -1, 0, 0);
    
    _nodes.insert(_nodes.begin() + bestIndex, {region.x, region.y + height, width});
    
    for (size_t i = bestIndex + 1; i < _nodes.size(); ++i) {
        glm::ivec3& node = _nodes[i];
        const glm::ivec3& prev = _nodes[i-1];
        
        if (node.x < (prev.x + prev.z)) {
            int shrink = prev.x + prev.z - node.x;
            node.x += shrink;
            node.z -= shrink;
            if (node.z <= 0) {
                _nodes.erase(_nodes.begin() + i);
                --i;
            }
            else
                break;
        }
        else
            break;
    }
    atlasMerge();
    _nUsed += width * height;
    return region;

}

void TextureAtlas::setRegion(const glm::ivec4 region, void* data, int stride) {
    setRegion(region.x, region.y, region.z, region.w, data, stride);
}

void TextureAtlas::setRegion(int x, int y, int width, int height, void* data, int stride) {
    ghoul_assert(x > 0, "x argument out of bounds");
    ghoul_assert(x < (_width - 1), "x argument out of bounds");
    ghoul_assert((x + width) <= (_width - 1), "x arguments out of bounds");

    ghoul_assert(y > 0, "y argument out of bounds");
    ghoul_assert(y < (_height - 1), "y argument out of bounds");
    ghoul_assert((y + height) <= (_height - 1), "y argument out of bounds");
    
    for (int i = 0; i < height; ++i) {
        void* dst = _data + ((y + i) * _width + x) * sizeof(char) * _depth;
        void* src = reinterpret_cast<unsigned char*>(data) + (i * stride) * sizeof(char);
        size_t nBytes = width * sizeof(char) * _depth;
        
        memcpy(dst, src, nBytes);
    }
}

int TextureAtlas::atlasFit(size_t index, int width, int height) {
    int x = _nodes[index].x;
    int y = _nodes[index].y;
    int width_left = width;
    size_t i = index;
    
    if ((x + width) > (_width - 1))
        return -1;
    while (width_left > 0) {
        const glm::ivec3& node = _nodes[i];
        
        if (node.y > y)
            y = node.y;
        if ((y + height) > (_height - 1))
            return -1;
        width_left -= node.z;
        ++i;
    }
    return y;
}

void TextureAtlas::atlasMerge() {
    for (size_t i = 0; i < _nodes.size() - 1; ++i) {
        glm::ivec3& node = _nodes[i];
        const glm::ivec3& next = _nodes[i+1];
        if (node.y == next.y) {
            node.z += next.z;
            _nodes.erase(_nodes.begin() + (i + 1));
            --i;
        }
    }
}



} // namespace opengl
} // namespace ghoul
