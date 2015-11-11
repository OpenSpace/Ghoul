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

#include <ghoul/opengl/texture.h>

using glm::detail::int8;
using glm::detail::int16;
using glm::detail::int32;
using glm::detail::uint8;
using glm::detail::uint16;
using glm::detail::uint32;
using glm::tvec2;
using glm::tvec3;
using glm::tvec4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::size3_t;
using glm::compMul;
using std::numeric_limits;

namespace ghoul {
namespace opengl {

Texture::Texture(glm::size3_t dimensions, Format format, GLint internalFormat,
                 GLenum dataType, FilterMode filter, WrappingMode wrapping)
    : _dimensions(std::move(dimensions))
    , _format(format)
    , _internalFormat(internalFormat)
    , _dataType(dataType)
    , _filter(filter)
    , _wrapping(wrapping)
    , _mipMapLevel(8)
    , _anisotropyLevel(-1.f)
    , _hasOwnershipOfData(false)
    , _pixels(0)
{
    initialize(true);
}

Texture::Texture(void* data, glm::size3_t dimensions, Format format,
                 GLint internalFormat, GLenum dataType, FilterMode filter,
                 WrappingMode wrapping)
     : _dimensions(std::move(dimensions))
     , _format(format)
     , _internalFormat(internalFormat)
     , _dataType(dataType)
     , _filter(filter)
     , _wrapping(wrapping)
     , _mipMapLevel(8)
     , _anisotropyLevel(-1.f)
     , _hasOwnershipOfData(true)
     , _pixels(data)
{
    initialize(false);
}

Texture::~Texture() {
    if (_id)
        glDeleteTextures(1, &_id);

    if (_hasOwnershipOfData)
        delete[] (GLubyte*)_pixels;
}

void Texture::initialize(bool allocateData) {
    determineTextureType();
    calculateBytesPerPixel();
    generateId();
    if (allocateData)
        allocateMemory();
    applyFilter();
    applyWrapping();
}

void Texture::allocateMemory() {
    size_t arraySize = compMul(_dimensions) * _bpp;
    _pixels = new GLubyte[arraySize];
}

void Texture::destroyMemory() {
    delete[] static_cast<GLubyte*>(_pixels);
    _pixels = 0;
}

void Texture::generateId() {
    _id = 0;
    glGenTextures(1, &_id);
}

void Texture::enable() const {
    glEnable(_type);
}

void Texture::disable()  const {
    glDisable(_type);
}

void Texture::bind() const {
    glBindTexture(_type, _id);
}

Texture::operator GLuint() const {
    return _id;
}

const std::string& Texture::getName() const {
    return _name;
}

void Texture::setName(std::string name) {
    _name = std::move(name);
}

GLenum Texture::type() const {
    return _type;
}

const glm::size3_t& Texture::dimensions() const {
    return _dimensions;
}

void Texture::setDimensions(size3_t dimensions) {
    _dimensions = std::move(dimensions);
}

size_t Texture::width() const {
    return _dimensions.x;
}

size_t Texture::height() const {
    return _dimensions.y;
}

size_t Texture::depth() const {
    return _dimensions.z;
}

Texture::Format Texture::format() const {
    return _format;
}

void Texture::setFormat(Texture::Format format) {
    _format = format;
}

GLint Texture::internalFormat() const {
    return _internalFormat;
}

void Texture::setInternalFormat(GLint internalFormat) {
    _internalFormat = internalFormat;
}

Texture::FilterMode Texture::filter() const {
    return _filter;
}

void Texture::setFilter(FilterMode filter) {
    _filter = filter;
    applyFilter();
}

void Texture::applyFilter() {
    bind();

    switch(_filter) {
        case FilterMode::Nearest:
            glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case FilterMode::Linear:
            glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case FilterMode::AnisotropicMipMap: {
                glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(_type, GL_TEXTURE_MAX_LEVEL, _mipMapLevel - 1);
                glGenerateMipmap(_type);
                if (_anisotropyLevel == -1.f) {
                    GLfloat maxTextureAnisotropy = 1.0;
                    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,
                        &maxTextureAnisotropy);
                    _anisotropyLevel = maxTextureAnisotropy;
                }
                glTexParameterf(_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, _anisotropyLevel);
                glTexParameteri(_type, GL_TEXTURE_BASE_LEVEL, 0);
                break;
            }
        default:
            assert(false);
    }
}

GLenum Texture::dataType() const {
    return _dataType;
}

void Texture::setDataType(GLenum dataType) {
    _dataType = dataType;
    calculateBytesPerPixel();
}

size_t Texture::expectedPixelDataSize() const {
    if (_pixels)
        return compMul(_dimensions) * _bpp;
    else
        return 0;
}

size_t Texture::numberOfChannels() const {
    switch (_format) {
        case Format::Red:
        case Format::DepthComponent:
            return 1;
            break;
        case Format::RG:
            return 2;
            break;
        case Format::RGB:
        case Format::BGR:
            return 3;
            break;
        case Format::RGBA:
        case Format::BGRA:
            return 4;
            break;
    }
    return 0;
}

GLubyte Texture::bytesPerPixel() const {
    return _bpp;
}

void Texture::setType(GLenum type) {
    _type = type;
}

void Texture::setDataOwnership(bool hasOwnership) {
    _hasOwnershipOfData = hasOwnership;
}

bool Texture::dataOwnership() const {
    return _hasOwnershipOfData;
}

const void* Texture::pixelData() const {
    return _pixels;
}

void Texture::setPixelData(void* pixels, bool takeOwnership) {
    if (_hasOwnershipOfData)
        delete (GLubyte*) _pixels;
    _hasOwnershipOfData = takeOwnership;
    _pixels = pixels;
}

bool Texture::isResident() const {
    GLint resident;
    glGetTexParameteriv(_type, GL_TEXTURE_RESIDENT, &resident);
    return (resident == GL_TRUE);
}

void Texture::setWrapping(WrappingMode wrapping) {
    _wrapping = wrapping;
    applyWrapping();
}

Texture::WrappingMode Texture::wrapping() const {
    return _wrapping;
}

void Texture::setMipMapLevel(int mipMapLevel) {
    _mipMapLevel = mipMapLevel;
    applyFilter();
}

void Texture::applyWrapping() {
    bind();
    GLint wrapping = GLint(_wrapping);
    switch (_type) {
        case GL_TEXTURE_3D:
            glTexParameteri(_type, GL_TEXTURE_WRAP_R, wrapping);
        case GL_TEXTURE_2D:
            glTexParameteri(_type, GL_TEXTURE_WRAP_T, wrapping);
        case GL_TEXTURE_1D:
            glTexParameteri(_type, GL_TEXTURE_WRAP_S, wrapping);
            break;
        default:
            assert(false);
    }
}

void Texture::uploadTexture() {
    bind();
    switch (_type) {
        case GL_TEXTURE_1D:
            glTexImage1D(_type, 0, _internalFormat,
                         GLsizei(_dimensions.x), 0, GLint(_format), _dataType, _pixels);
            break;
        case GL_TEXTURE_2D:
            glTexImage2D(_type, 0, _internalFormat,
                         GLsizei(_dimensions.x), GLsizei(_dimensions.y), 0, GLint(_format),
                         _dataType, _pixels);
            break;
        case GL_TEXTURE_3D:
            glTexImage3D(_type, 0, _internalFormat,
                         GLsizei(_dimensions.x), GLsizei(_dimensions.y),
                         GLsizei(_dimensions.z), 0, GLint(_format), _dataType, _pixels);
            break;
        default:
            assert(false);
    }
}

void Texture::downloadTexture() {
    bind();
    if (!_pixels)
        allocateMemory();
    glGetTexImage(_type, 0, GLint(_format), _dataType, _pixels);
}

void Texture::determineTextureType() {
    if (_dimensions.z == 1) {
        if (_dimensions.y == 1)
            _type = GL_TEXTURE_1D;
        else
            _type = GL_TEXTURE_2D;
    }
    else
        _type = GL_TEXTURE_3D;
}

vec4 Texture::texelAsFloat(size_t x) const {
    assert(x < compMul(_dimensions));
    assert(_type == GL_TEXTURE_1D);

    vec4 result(0.f);
    switch (_format) {
        case Format::Red:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    uint8 t = texel<uint8>(x);
                    result.r = static_cast<float>(t) / numeric_limits<uint8>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    int8 t = texel<int8>(x);
                    result.r = static_cast<float>(t) / numeric_limits<int8>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    uint16 t = texel<uint16>(x);
                    result.r = static_cast<float>(t) / numeric_limits<uint16>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    int16 t = texel<int16>(x);
                    result.r = static_cast<float>(t) / numeric_limits<int16>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    uint32 t = texel<uint32>(x);
                    result.r = static_cast<float>(t) / numeric_limits<uint32>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    int32 t = texel<int32>(x);
                    result.r = static_cast<float>(t) / numeric_limits<int32>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    float t = texel<float>(x);
                    result.r = t;
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::RG:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec2<uint8> t = texel<tvec2<uint8> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    tvec2<int8> t = texel<tvec2<int8> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec2<uint16> t = texel<tvec2<uint16> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    tvec2<int16> t = texel<tvec2<int16> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec2<uint32> t = texel<tvec2<uint32> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    tvec2<int32> t = texel<tvec2<int32> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    vec2 t = texel<vec2>(x);
                    result.r = t.r;
                    result.g = t.g;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::RGB:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec3<uint8> t = texel<tvec3<uint8> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    tvec3<int8> t = texel<tvec3<int8> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec3<uint16> t = texel<tvec3<uint16> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    tvec3<int16> t = texel<tvec3<int16> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec3<uint32> t = texel<tvec3<uint32> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    tvec3<int32> t = texel<tvec3<int32> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    vec3 t = texel<vec3>(x);
                    result.r = t.r;
                    result.g = t.g;
                    result.b = t.b;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::BGR:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec3<uint8> t = texel<tvec3<uint8> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    tvec3<int8> t = texel<tvec3<int8> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec3<uint16> t = texel<tvec3<uint16> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    tvec3<int16> t = texel<tvec3<int16> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec3<uint32> t = texel<tvec3<uint32> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    tvec3<int32> t = texel<tvec3<int32> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    vec3 t = texel<vec3>(x);
                    result.r = t.b;
                    result.g = t.g;
                    result.b = t.r;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::RGBA:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec4<uint8> t = texel<tvec4<uint8> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint8>::max();
                    break;
                }
                case GL_BYTE: {
                    tvec4<int8> t = texel<tvec4<int8> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int8>::max();
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec4<uint16> t = texel<tvec4<uint16> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint16>::max();
                    break;
                }
                case GL_SHORT: {
                    tvec4<int16> t = texel<tvec4<int16> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int16>::max();
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec4<uint32> t = texel<tvec4<uint32> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint32>::max();
                    break;
                }
                case GL_INT: {
                    tvec4<int32> t = texel<tvec4<int32> >(x);
                    result.r = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int32>::max();
                    break;
                }
                case GL_FLOAT: {
                    vec4 t = texel<vec4>(x);
                    result.r = t.r;
                    result.g = t.g;
                    result.b = t.b;
                    result.a = t.a;
                    break;
                }
            }
            break;
        case Format::BGRA:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec4<uint8> t = texel<tvec4<uint8> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint8>::max();
                    break;
                }
                case GL_BYTE: {
                    tvec4<int8> t = texel<tvec4<int8> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int8>::max();
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec4<uint16> t = texel<tvec4<uint16> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint16>::max();
                    break;
                }
                case GL_SHORT: {
                    tvec4<int16> t = texel<tvec4<int16> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int16>::max();
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec4<uint32> t = texel<tvec4<uint32> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint32>::max();
                    break;
                }
                case GL_INT: {
                    tvec4<int32> t = texel<tvec4<int32> >(x);
                    result.r = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int32>::max();
                    break;
                }
                case GL_FLOAT: {
                    vec4 t = texel<vec4>(x);
                    result.r = t.b;
                    result.g = t.g;
                    result.b = t.r;
                    result.a = t.a;
                    break;
                }
            }
            break;
        case Format::DepthComponent:
            break;
    }
    return result;
}

vec4 Texture::texelAsFloat(size_t x, size_t y) const {
    assert((y * _dimensions.x) + x < compMul(_dimensions));
    assert(_type == GL_TEXTURE_2D);

    vec4 result(0.f);
    switch (_format) {
        case Format::Red:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    uint8 t = texel<uint8>(x,y);
                    result.r = static_cast<float>(t) / numeric_limits<uint8>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    int8 t = texel<int8>(x,y);
                    result.r = static_cast<float>(t) / numeric_limits<int8>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    uint16 t = texel<uint16>(x,y);
                    result.r = static_cast<float>(t) / numeric_limits<uint16>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    int16 t = texel<int16>(x,y);
                    result.r = static_cast<float>(t) / numeric_limits<int16>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    uint32 t = texel<uint32>(x,y);
                    result.r = static_cast<float>(t) / numeric_limits<uint32>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    int32 t = texel<int32>(x,y);
                    result.r = static_cast<float>(t) / numeric_limits<int32>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    float t = texel<float>(x,y);
                    result.r = t;
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::RG:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec2<uint8> t = texel<tvec2<uint8> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    tvec2<int8> t = texel<tvec2<int8> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec2<uint16> t = texel<tvec2<uint16> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    tvec2<int16> t = texel<tvec2<int16> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec2<uint32> t = texel<tvec2<uint32> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    tvec2<int32> t = texel<tvec2<int32> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    vec2 t = texel<vec2>(x,y);
                    result.r = t.r;
                    result.g = t.g;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::RGB:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec3<uint8> t = texel<tvec3<uint8> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    tvec3<int8> t = texel<tvec3<int8> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec3<uint16> t = texel<tvec3<uint16> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    tvec3<int16> t = texel<tvec3<int16> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec3<uint32> t = texel<tvec3<uint32> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    tvec3<int32> t = texel<tvec3<int32> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    vec3 t = texel<vec3>(x,y);
                    result.r = t.r;
                    result.g = t.g;
                    result.b = t.b;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::BGR:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec3<uint8> t = texel<tvec3<uint8> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    tvec3<int8> t = texel<tvec3<int8> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec3<uint16> t = texel<tvec3<uint16> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    tvec3<int16> t = texel<tvec3<int16> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec3<uint32> t = texel<tvec3<uint32> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    tvec3<int32> t = texel<tvec3<int32> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    vec3 t = texel<vec3>(x,y);
                    result.r = t.b;
                    result.g = t.g;
                    result.b = t.r;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::RGBA:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec4<uint8> t = texel<tvec4<uint8> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint8>::max();
                    break;
                }
                case GL_BYTE: {
                    tvec4<int8> t = texel<tvec4<int8> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int8>::max();
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec4<uint16> t = texel<tvec4<uint16> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint16>::max();
                    break;
                }
                case GL_SHORT: {
                    tvec4<int16> t = texel<tvec4<int16> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int16>::max();
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec4<uint32> t = texel<tvec4<uint32> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint32>::max();
                    break;
                }
                case GL_INT: {
                    tvec4<int32> t = texel<tvec4<int32> >(x,y);
                    result.r = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int32>::max();
                    break;
                }
                case GL_FLOAT: {
                    vec4 t = texel<vec4>(x,y);
                    result.r = t.r;
                    result.g = t.g;
                    result.b = t.b;
                    result.a = t.a;
                    break;
                }
            }
            break;
        case Format::BGRA:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec4<uint8> t = texel<tvec4<uint8> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint8>::max();
                    break;
                }
                case GL_BYTE: {
                    tvec4<int8> t = texel<tvec4<int8> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int8>::max();
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec4<uint16> t = texel<tvec4<uint16> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint16>::max();
                    break;
                }
                case GL_SHORT: {
                    tvec4<int16> t = texel<tvec4<int16> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int16>::max();
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec4<uint32> t = texel<tvec4<uint32> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint32>::max();
                    break;
                }
                case GL_INT: {
                    tvec4<int32> t = texel<tvec4<int32> >(x,y);
                    result.r = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int32>::max();
                    break;
                }
                case GL_FLOAT: {
                    vec4 t = texel<vec4>(x,y);
                    result.r = t.b;
                    result.g = t.g;
                    result.b = t.r;
                    result.a = t.a;
                    break;
                }
            }
            break;
        case Format::DepthComponent:
            break;
    }
    return result;
}

vec4 Texture::texelAsFloat(size_t x, size_t y, size_t z) const {
    assert((z * _dimensions.x * _dimensions.y) + (y * _dimensions.x) + x < compMul(_dimensions));
    assert(_type == GL_TEXTURE_3D);

    vec4 result(0.f);
    switch (_format) {
        case Format::Red:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    uint8 t = texel<uint8>(x,y,z);
                    result.r = static_cast<float>(t) / numeric_limits<uint8>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    int8 t = texel<int8>(x,y,z);
                    result.r = static_cast<float>(t) / numeric_limits<int8>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    uint16 t = texel<uint16>(x,y,z);
                    result.r = static_cast<float>(t) / numeric_limits<uint16>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    int16 t = texel<int16>(x,y,z);
                    result.r = static_cast<float>(t) / numeric_limits<int16>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    uint32 t = texel<uint32>(x,y,z);
                    result.r = static_cast<float>(t) / numeric_limits<uint32>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    int32 t = texel<int32>(x,y,z);
                    result.r = static_cast<float>(t) / numeric_limits<int32>::max();
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    float t = texel<float>(x,y,z);
                    result.r = t;
                    result.g = 0.f;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::RG:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec2<uint8> t = texel<tvec2<uint8> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    tvec2<int8> t = texel<tvec2<int8> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec2<uint16> t = texel<tvec2<uint16> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    tvec2<int16> t = texel<tvec2<int16> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec2<uint32> t = texel<tvec2<uint32> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    tvec2<int32> t = texel<tvec2<int32> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    vec2 t = texel<vec2>(x,y,z);
                    result.r = t.r;
                    result.g = t.g;
                    result.b = 0.f;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::RGB:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec3<uint8> t = texel<tvec3<uint8> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    tvec3<int8> t = texel<tvec3<int8> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec3<uint16> t = texel<tvec3<uint16> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    tvec3<int16> t = texel<tvec3<int16> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec3<uint32> t = texel<tvec3<uint32> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    tvec3<int32> t = texel<tvec3<int32> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    vec3 t = texel<vec3>(x,y,z);
                    result.r = t.r;
                    result.g = t.g;
                    result.b = t.b;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::BGR:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec3<uint8> t = texel<tvec3<uint8> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_BYTE: {
                    tvec3<int8> t = texel<tvec3<int8> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec3<uint16> t = texel<tvec3<uint16> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_SHORT: {
                    tvec3<int16> t = texel<tvec3<int16> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec3<uint32> t = texel<tvec3<uint32> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_INT: {
                    tvec3<int32> t = texel<tvec3<int32> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.a = 1.f;
                    break;
                }
                case GL_FLOAT: {
                    vec3 t = texel<vec3>(x,y,z);
                    result.r = t.b;
                    result.g = t.g;
                    result.b = t.r;
                    result.a = 1.f;
                    break;
                }
            }
            break;
        case Format::RGBA:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec4<uint8> t = texel<tvec4<uint8> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint8>::max();
                    break;
                }
                case GL_BYTE: {
                    tvec4<int8> t = texel<tvec4<int8> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int8>::max();
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec4<uint16> t = texel<tvec4<uint16> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint16>::max();
                    break;
                }
                case GL_SHORT: {
                    tvec4<int16> t = texel<tvec4<int16> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int16>::max();
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec4<uint32> t = texel<tvec4<uint32> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint32>::max();
                    break;
                }
                case GL_INT: {
                    tvec4<int32> t = texel<tvec4<int32> >(x,y,z);
                    result.r = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int32>::max();
                    break;
                }
                case GL_FLOAT: {
                    vec4 t = texel<vec4>(x,y,z);
                    result.r = t.r;
                    result.g = t.g;
                    result.b = t.b;
                    result.a = t.a;
                    break;
                }
            }
            break;
        case Format::BGRA:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE: {
                    tvec4<uint8> t = texel<tvec4<uint8> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint8>::max();
                    break;
                }
                case GL_BYTE: {
                    tvec4<int8> t = texel<tvec4<int8> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<int8>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int8>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int8>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int8>::max();
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tvec4<uint16> t = texel<tvec4<uint16> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint16>::max();
                    break;
                }
                case GL_SHORT: {
                    tvec4<int16> t = texel<tvec4<int16> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<int16>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int16>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int16>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int16>::max();
                    break;
                }
                case GL_UNSIGNED_INT: {
                    tvec4<uint32> t = texel<tvec4<uint32> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<uint32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<uint32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<uint32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<uint32>::max();
                    break;
                }
                case GL_INT: {
                    tvec4<int32> t = texel<tvec4<int32> >(x,y,z);
                    result.r = static_cast<float>(t.b) / numeric_limits<int32>::max();
                    result.g = static_cast<float>(t.g) / numeric_limits<int32>::max();
                    result.b = static_cast<float>(t.r) / numeric_limits<int32>::max();
                    result.a = static_cast<float>(t.a) / numeric_limits<int32>::max();
                    break;
                }
                case GL_FLOAT: {
                    vec4 t = texel<vec4>(x,y,z);
                    result.r = t.b;
                    result.g = t.g;
                    result.b = t.r;
                    result.a = t.a;
                    break;
                }
            }
            break;
        case Format::DepthComponent:
            break;
    }
    return result;
}

vec4 Texture::texelAsFloat(const glm::size2_t& pos) const {
    return texelAsFloat(pos.x, pos.y);
}

vec4 Texture::texelAsFloat(const glm::size3_t& pos) const {
    return texelAsFloat(pos.x, pos.y, pos.z);
}

void Texture::calculateBytesPerPixel() {
    size_t numChannels = numberOfChannels();
    int szType = 0;
    switch (_dataType) {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
            szType = 1;
            break;
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            szType = 2;
            break;
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            szType = 4;
            break;
        default:
            assert(false);
    }

    _bpp = static_cast<GLubyte>(szType * numChannels);
}

} // opengl
} // ghoul
