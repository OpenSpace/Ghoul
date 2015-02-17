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

#include <glm/gtx/component_wise.hpp>
#include <cassert>

namespace ghoul {
namespace opengl {

template <class T>
T& Texture::texel(size_t x) {
    assert(sizeof(T) == _bpp);
    assert(x < glm::compMul(_dimensions));
    assert(_type == GL_TEXTURE_1D);
    return (reinterpret_cast<T*>(_pixels))[x];
}

template <class T>
const T& Texture::texel(size_t x) const {
    assert(sizeof(T) == _bpp);
    assert(x < glm::compMul(_dimensions));
    assert(_type == GL_TEXTURE_1D);
    return (reinterpret_cast<T*>(_pixels))[x];
}

template <class T>
T& Texture::texel(size_t x, size_t y) {
    assert(sizeof(T) == _bpp);
    assert((y * _dimensions.x) + x < glm::compMul(_dimensions));
    assert(_type == GL_TEXTURE_2D);
    return (reinterpret_cast<T*>(_pixels)[y * _dimensions.x + x]);
}

template<class T>
const T& Texture::texel(size_t x, size_t y) const {
    assert(sizeof(T) == _bpp);
    assert((y * _dimensions.x) + x < glm::compMul(_dimensions));
    assert(_type == GL_TEXTURE_2D);
    return (reinterpret_cast<T*>(_pixels)[y * _dimensions.x + x]);
}

template <class T>
T& Texture::texel(const glm::size2_t& position) {
    assert(sizeof(T) == _bpp);
    assert((position.y * _dimensions.x) + position.x < glm::compMul(_dimensions));
    assert(_type == GL_TEXTURE_2D);
    return (reinterpret_cast<T*>(_pixels)[(position.y * _dimensions.x) + position.x]);
}

template<class T>
const T& Texture::texel(const glm::size2_t& position) const {
    assert(sizeof(T) == _bpp);
    assert((position.y * _dimensions.x) + position.x < glm::compMul(_dimensions));
    assert(_type == GL_TEXTURE_2D);
    return (reinterpret_cast<T*>(_pixels)[(position.y * _dimensions.x) + position.x]);
}

template <class T>
T& Texture::texel(size_t x, size_t y, size_t z) {
    assert(sizeof(T) == _bpp);
    assert((z * _dimensions.x * _dimensions.y) +
            (y * _dimensions.x) + x < glm::compMul(_dimensions));
    assert(_type == GL_TEXTURE_3D);
    return (reinterpret_cast<T*>(_pixels)[(z * _dimensions.x * _dimensions.y) +
                                          (y * _dimensions.x) + x]);
}

template<class T>
const T& Texture::texel(size_t x, size_t y, size_t z) const {
    assert(sizeof(T) == _bpp);
    assert((z * _dimensions.x * _dimensions.y) +
        (y * _dimensions.x) + x < glm::compMul(_dimensions));
    assert(_type == GL_TEXTURE_3D);
    return (reinterpret_cast<T*>(_pixels)[(z * _dimensions.x * _dimensions.y) +
        (y * _dimensions.x) + x]);
}

template <class T>
T& Texture::texel(const glm::size3_t& position) {
    assert(sizeof(T) == _bpp);
    assert((position.z * _dimensions.x * _dimensions.y) +
        (position.y * _dimensions.x) + position.x < glm::compMul(_dimensions));
    assert(_type == GL_TEXTURE_3D);
    return (reinterpret_cast<T*>(_pixels)[(position.z * _dimensions.x * _dimensions.y) +
        (position.y * _dimensions.x) + position.x]);
}

template<class T>
const T& Texture::texel(const glm::size3_t& position) const {
    assert(sizeof(T) == _bpp);
    assert((position.z * _dimensions.x * _dimensions.y) +
        (position.y * _dimensions.x) + position.x < glm::compMul(_dimensions));
    assert(_type == GL_TEXTURE_3D);
    return (reinterpret_cast<T*>(_pixels)[(position.z * _dimensions.x * _dimensions.y) +
        (position.y * _dimensions.x) + position.x]);
}

} // namespace opengl
} // namespace ghoul
