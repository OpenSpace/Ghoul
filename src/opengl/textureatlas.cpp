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

TextureAtlas::TextureAtlas(glm::ivec3 size)
    : _size(std::move(size))
    , _nUsed(0)
    , _data(nullptr)
{
    // Limitations to the depth are due to the fact that the atlas is represented by
    // a single texture on the GPU (which only allows up to four channels)
    ghoul_assert(_size.x > 4, "Width has to be bigger than 4");
    ghoul_assert(_size.y > 4, "Height has to be bigger than 4");
    ghoul_assert(_size.z >= 1, "Depth has to be positive");
    ghoul_assert(_size.z <= 4, "Depth has to be smaller or equal to 4");
    
    _nodes.emplace_back(1, 1, _size.x - 2);
    _data = new unsigned char[_size.x * _size.y * _size.z];
    std::memset(_data, 0, _size.x * _size.y * _size.z);
    
    Texture::Format format;
    switch (_size.z) {
        case 1:
            format = Texture::Format::Red;
            break;
        case 2:
            format = Texture::Format::RG;
            break;
        case 3:
            format = Texture::Format::RGB;
            break;
        case 4:
            format = Texture::Format::RGBA;
            break;
    }
    GLuint internalFormat = format;
    GLenum dataType = GL_UNSIGNED_BYTE;
    
#ifdef GL_UNSIGNED_INT_8_8_8_8_REV
    if (_size.z == 4) {
        internalFormat = GL_BGRA;
        dataType = GL_UNSIGNED_INT_8_8_8_8_REV;
    }
#endif

    _texture = new Texture(
        _size,
        format,
        internalFormat,
        dataType
    );
}

TextureAtlas::TextureAtlas(int width, int height, int depth)
    : TextureAtlas(glm::ivec3(width, height, depth))
{}

TextureAtlas::~TextureAtlas() {
    delete _texture;
    delete[] _data;
}

int TextureAtlas::width() const {
    return _size.x;
}

int TextureAtlas::height() const {
    return _size.y;
}

int TextureAtlas::depth() const {
    return _size.z;
}

const Texture& TextureAtlas::texture() const {
    return *_texture;
}

void TextureAtlas::upload() {
    _texture->setPixelData(_data, false);
    _texture->bind();
    _texture->uploadTexture();
}

void TextureAtlas::clear() {
    _nodes.clear();
    _nodes.emplace_back(1, 1 , _size.x - 2);

    _nUsed = 0;
    
    std::memset(_data, 0, _size.x * _size.y * _size.z);
}

glm::ivec4 TextureAtlas::newRegion(int width, int height) {
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

void TextureAtlas::setRegionData(const glm::ivec4& region, void* data) {
    int x = region.x;
    int y = region.y;
    int width = region.z;
    int height = region.w;
    
    ghoul_assert(x > 0, "x argument out of bounds");
    ghoul_assert(x < (_size.x - 1), "x argument out of bounds");
    ghoul_assert((x + width) <= (_size.x - 1), "x arguments out of bounds");
    
    ghoul_assert(y > 0, "y argument out of bounds");
    ghoul_assert(y < (_size.y - 1), "y argument out of bounds");
    ghoul_assert((y + height) <= (_size.y - 1), "y argument out of bounds");
    
    for (int i = 0; i < height; ++i) {
        void* dst = _data + ((y + i) * _size.x + x) * sizeof(char) * _size.z;
        void* src = reinterpret_cast<unsigned char*>(data) + (i * width) * sizeof(char);
        size_t nBytes = width * sizeof(char) * _size.z;
        
        memcpy(dst, src, nBytes);
    }
}

void TextureAtlas::setRegionData(int x, int y, int width, int height, void* data) {
    setRegionData(glm::ivec4(x, y, width, height), data);
}

int TextureAtlas::atlasFit(size_t index, int width, int height) {
    int x = _nodes[index].x;
    int y = _nodes[index].y;
    
    if ((x + width) > (_size.x - 1))
        return -1;
    while (width > 0) {
        const glm::ivec3& node = _nodes[index];
        
        if (node.y > y)
            y = node.y;
        if ((y + height) > (_size.y - 1))
            return -1;
        width -= node.z;
        ++index;
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
