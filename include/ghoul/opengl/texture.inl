/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2025                                                               *
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

#include <ghoul/misc/assert.h>
#include <glm/gtx/component_wise.hpp>

namespace ghoul::opengl {

template <class T>
T& Texture::texel(unsigned int x) {
    ghoul_assert(sizeof(T) == _bpp, "Size of T must be equal to texel size");
    ghoul_assert(x < _dimensions.x, "x must be smaller than the width of the Texture");
    ghoul_assert(_type == GL_TEXTURE_1D, "Function must be called on a 1D texture");
    return (reinterpret_cast<T*>(_pixels))[x];
}

template <class T>
const T& Texture::texel(unsigned int x) const {
    ghoul_assert(sizeof(T) == _bpp, "Size of T must be equal to texel size");
    ghoul_assert(x < _dimensions.x, "x must be smaller than the width of the Texture");
    ghoul_assert(_type == GL_TEXTURE_1D, "Function must be called on a 1D texture");
    return (reinterpret_cast<T*>(_pixels))[x];
}

template <class T>
T& Texture::texel(unsigned int x, unsigned int y) {
    ghoul_assert(sizeof(T) == _bpp, "Size of T must be equal to texel size");
    ghoul_assert(x < _dimensions.x, "x must be smaller than the width of the Texture");
    ghoul_assert(y < _dimensions.y, "y must be smaller than the height of the Texture");
    ghoul_assert(_type == GL_TEXTURE_2D, "Function must be called on a 2D texture");
    return (reinterpret_cast<T*>(_pixels)[y * _dimensions.x + x]);
}

template <class T>
const T& Texture::texel(unsigned int x, unsigned int y) const {
    ghoul_assert(sizeof(T) == _bpp, "Size of T must be equal to texel size");
    ghoul_assert(x < _dimensions.x, "x must be smaller than the width of the Texture");
    ghoul_assert(y < _dimensions.y, "y must be smaller than the height of the Texture");
    ghoul_assert(_type == GL_TEXTURE_2D, "Function must be called on a 2D texture");
    return (reinterpret_cast<T*>(_pixels)[y * _dimensions.x + x]);
}

template <class T>
T& Texture::texel(const glm::uvec2& position) {
    ghoul_assert(sizeof(T) == _bpp, "Size of T must be equal to texel size");
    ghoul_assert(
        position.x < _dimensions.x,
        "x must be smaller than the width of the Texture"
    );
    ghoul_assert(
        position.y < _dimensions.y,
        "y must be smaller than the height of the Texture"
    );
    ghoul_assert(_type == GL_TEXTURE_2D, "Function must be called on a 2D texture");
    return (reinterpret_cast<T*>(_pixels)[(position.y * _dimensions.x) + position.x]);
}

template <class T>
const T& Texture::texel(const glm::uvec2& position) const {
    ghoul_assert(sizeof(T) == _bpp, "Size of T must be equal to texel size");
    ghoul_assert(
        position.x < _dimensions.x,
        "x must be smaller than the width of the Texture"
    );
    ghoul_assert(
        position.y < _dimensions.y,
        "y must be smaller than the height of the Texture"
    );
    ghoul_assert(_type == GL_TEXTURE_2D, "Function must be called on a 2D texture");
    return (reinterpret_cast<T*>(_pixels)[(position.y * _dimensions.x) + position.x]);
}

template <class T>
T& Texture::texel(unsigned int x, unsigned int y, unsigned int z) {
    ghoul_assert(sizeof(T) == _bpp, "Size of T must be equal to texel size");
    ghoul_assert(x < _dimensions.x, "x must be smaller than the width of the Texture");
    ghoul_assert(y < _dimensions.y, "y must be smaller than the height of the Texture");
    ghoul_assert(z < _dimensions.z, "z must be smaller than the depth of the Texture");
    ghoul_assert(_type == GL_TEXTURE_3D, "Function must be called on a 3D texture");
    return (reinterpret_cast<T*>(_pixels)[(z * _dimensions.x * _dimensions.y) +
                                          (y * _dimensions.x) + x]);
}

template <class T>
const T& Texture::texel(unsigned int x, unsigned int y, unsigned int z) const {
    ghoul_assert(sizeof(T) == _bpp, "Size of T must be equal to texel size");
    ghoul_assert(x < _dimensions.x, "x must be smaller than the width of the Texture");
    ghoul_assert(y < _dimensions.y, "y must be smaller than the height of the Texture");
    ghoul_assert(z < _dimensions.z, "z must be smaller than the depth of the Texture");
    ghoul_assert(_type == GL_TEXTURE_3D, "Function must be called on a 3D texture");
    return (reinterpret_cast<T*>(_pixels)[(z * _dimensions.x * _dimensions.y) +
        (y * _dimensions.x) + x]);
}

template <class T>
T& Texture::texel(const glm::uvec3& position) {
    ghoul_assert(sizeof(T) == _bpp, "Size of T must be equal to texel size");
    ghoul_assert(
        position.x < _dimensions.x,
        "x must be smaller than the width of the Texture"
    );
    ghoul_assert(
        position.y < _dimensions.y,
        "y must be smaller than the height of the Texture"
    );
    ghoul_assert(
        position.z < _dimensions.z,
        "z must be smaller than the depth of the Texture"
    );
    ghoul_assert(_type == GL_TEXTURE_3D, "Function must be called on a 3D texture");
    return (reinterpret_cast<T*>(_pixels)[(position.z * _dimensions.x * _dimensions.y) +
        (position.y * _dimensions.x) + position.x]);
}

template <class T>
const T& Texture::texel(const glm::uvec3& position) const {
    ghoul_assert(sizeof(T) == _bpp, "Size of T must be equal to texel size");
    ghoul_assert(
        position.x < _dimensions.x,
        "x must be smaller than the width of the Texture"
    );
    ghoul_assert(
        position.y < _dimensions.y,
        "y must be smaller than the height of the Texture"
    );
    ghoul_assert(
        position.z < _dimensions.z,
        "z must be smaller than the depth of the Texture"
    );
    ghoul_assert(_type == GL_TEXTURE_3D, "Function must be called on a 3D texture");
    return (reinterpret_cast<T*>(_pixels)[(position.z * _dimensions.x * _dimensions.y) +
        (position.y * _dimensions.x) + position.x]);
}

} // namespace ghoul::opengl
