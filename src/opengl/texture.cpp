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
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR .OTHER LIABILITY, WHETHER IN AN ACTION OF *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <ghoul/opengl/texture.h>

#include <ghoul/misc/assert.h>
#include <cstring>

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
using glm::compMul;
using std::numeric_limits;

namespace ghoul::opengl {

#ifdef Debugging_Ghoul_Textures_Indices
int Texture::nextIndex = 0;
#endif // Debugging_Ghoul_Textures_Indices

Texture::Texture(glm::uvec3 dimensions, Format format, GLenum internalFormat,
                 GLenum dataType, FilterMode filter, WrappingMode wrapping,
                 AllocateData allocate, TakeOwnership takeOwnership)
    : _dimensions(std::move(dimensions))
    , _format(format)
    , _internalFormat(internalFormat)
    , _dataType(dataType)
    , _filter(filter)
    , _wrapping({ wrapping })
    , _hasOwnershipOfData(takeOwnership)
{
#ifdef Debugging_Ghoul_Textures_Indices
    index = nextIndex++;
#endif // Debugging_Ghoul_Textures_Indices

    ghoul_assert(_dimensions.x >= 1, "Element of dimensions must be bigger or equal 1");
    ghoul_assert(_dimensions.y >= 1, "Element of dimensions must be bigger or equal 1");
    ghoul_assert(_dimensions.z >= 1, "Element of dimensions must be bigger or equal 1");

    if (!allocate && takeOwnership) {
        _hasOwnershipOfData = TakeOwnership::No;
    }

    initialize(allocate);
}

Texture::Texture(void* data, glm::uvec3 dimensions, Format format, GLenum internalFormat,
                 GLenum dataType, FilterMode filter, WrappingMode wrapping)
    : _dimensions(std::move(dimensions))
    , _format(format)
    , _internalFormat(internalFormat)
    , _dataType(dataType)
    , _filter(filter)
    , _wrapping({ wrapping })
    , _hasOwnershipOfData(true)
    , _pixels(data)
{
#ifdef Debugging_Ghoul_Textures_Indices
    index = nextIndex++;
#endif // Debugging_Ghoul_Textures_Indices

    ghoul_assert(_dimensions.x >= 1, "Element of dimensions must be bigger or equal 1");
    ghoul_assert(_dimensions.y >= 1, "Element of dimensions must be bigger or equal 1");
    ghoul_assert(_dimensions.z >= 1, "Element of dimensions must be bigger or equal 1");

    initialize(false);
}

Texture::~Texture() {
    if (_id) {
        glDeleteTextures(1, &_id);
    }

    if (_hasOwnershipOfData) {
        destroyMemory();
    }
}

void Texture::initialize(bool allocateData) {
    // Determine texture type
    if (_dimensions.z == 1) {
        if (_dimensions.y == 1) {
            _type = GL_TEXTURE_1D;
        }
        else {
            _type = GL_TEXTURE_2D;
        }
    }
    else {
        _type = GL_TEXTURE_3D;
    }

    calculateBytesPerPixel();
    generateId();
    if (allocateData) {
        allocateMemory();
    }
    applyFilter();
    applyWrapping();
}

void Texture::allocateMemory() {
    unsigned int arraySize = compMul(_dimensions) * _bpp;
    _pixels = new GLubyte[arraySize];
    std::memset(_pixels, 0, arraySize);
}

void Texture::destroyMemory() {
    delete[] static_cast<GLubyte*>(_pixels);
    _pixels = nullptr;
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

const glm::uvec3& Texture::dimensions() const {
    return _dimensions;
}

void Texture::setDimensions(glm::uvec3 dimensions) {
    _dimensions = std::move(dimensions);
}

unsigned int Texture::width() const {
    return _dimensions.x;
}

unsigned int Texture::height() const {
    return _dimensions.y;
}

unsigned int Texture::depth() const {
    return _dimensions.z;
}

Texture::Format Texture::format() const {
    return _format;
}

void Texture::setFormat(Texture::Format format) {
    _format = format;
    calculateBytesPerPixel();
}

GLenum Texture::internalFormat() const {
    return _internalFormat;
}

void Texture::setInternalFormat(GLenum internalFormat) {
    _internalFormat = internalFormat;
}

Texture::FilterMode Texture::filter() const {
    return _filter;
}

void Texture::setFilter(FilterMode filter) {
    _filter = filter;
    applyFilter();
}

void Texture::setSwizzleMask(std::array<GLenum, 4> swizzleMask) {
    _swizzleMask = std::move(swizzleMask);
    _swizzleMaskChanged = true;
    applySwizzleMask();
}

void Texture::setDefaultSwizzleMask() {
    _swizzleMask = DefaultSwizzleMask;
    _swizzleMaskChanged = false;
}

std::array<GLenum, 4> Texture::swizzleMask() const {
    return _swizzleMask;
}

void Texture::applyFilter() {
    bind();

    switch (_filter) {
        case FilterMode::Nearest:
            glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case FilterMode::Linear:
            glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case FilterMode::LinearMipMap:
            glGenerateMipmap(_type);
            glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(_type, GL_TEXTURE_MAX_LEVEL, _mipMapLevel - 1);
            break;
        case FilterMode::AnisotropicMipMap:
        {
            glGenerateMipmap(_type);
            glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(_type, GL_TEXTURE_MAX_LEVEL, _mipMapLevel - 1);
            if (std::equal_to<>()(_anisotropyLevel, -1.f)) {
                GLfloat maxTextureAnisotropy = 1.0;
                glGetFloatv(
                    GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,
                    &maxTextureAnisotropy
                );
                _anisotropyLevel = maxTextureAnisotropy;
            }
            glTexParameterf(_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, _anisotropyLevel);
            glTexParameteri(_type, GL_TEXTURE_BASE_LEVEL, 0);
            break;
        }
        default:
            throw MissingCaseException();
    }
}

GLenum Texture::dataType() const {
    return _dataType;
}

void Texture::setDataType(GLenum dataType) {
    _dataType = dataType;
    calculateBytesPerPixel();
}

int Texture::expectedPixelDataSize() const {
    if (_pixels) {
        return compMul(_dimensions) * _bpp;
    }
    else {
        return 0;
    }
}

int Texture::numberOfChannels(Format format) {
    switch (format) {
        case Format::Red:
        case Format::DepthComponent:
            return 1;
        case Format::RG:
            return 2;
        case Format::RGB: // Intentional fallthrough
        case Format::BGR:
            return 3;
        case Format::RGBA: // Intentional fallthrough
        case Format::BGRA:
            return 4;
    }
    return 0;
}

int Texture::numberOfChannels() const {
    return numberOfChannels(_format);
}

GLubyte Texture::bytesPerPixel() const {
    return _bpp;
}

void Texture::setType(GLenum type) {
    ghoul_assert(
        (type == GL_TEXTURE_1D) || (type == GL_TEXTURE_2D) || (type == GL_TEXTURE_3D),
        "Type must be GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D"
    );
    _type = type;
}

void Texture::setDataOwnership(TakeOwnership hasOwnership) {
    _hasOwnershipOfData = hasOwnership;
}

bool Texture::dataOwnership() const {
    return _hasOwnershipOfData;
}

const void* Texture::pixelData() const {
    return _pixels;
}

void Texture::setPixelData(void* pixels, TakeOwnership takeOwnership) {
    if (_hasOwnershipOfData) {
        destroyMemory();
    }
    _hasOwnershipOfData = takeOwnership;
    _pixels = pixels;
    calculateBytesPerPixel();
}

bool Texture::isResident() const {
    GLint resident;
    glGetTexParameteriv(_type, GL_TEXTURE_RESIDENT, &resident);
    return (static_cast<GLboolean>(resident) == GL_TRUE);
}

void Texture::setWrapping(WrappingMode wrapping) {
    _wrapping = { wrapping, wrapping, wrapping };
    applyWrapping();
}

void Texture::setWrapping(WrappingModes wrapping) {
    _wrapping = wrapping;
    applyWrapping();
}

Texture::WrappingModes Texture::wrapping() const {
    return _wrapping;
}

void Texture::setMipMapLevel(int mipMapLevel) {
    _mipMapLevel = mipMapLevel;
    applyFilter();
}

void Texture::applyWrapping() {
    bind();
    switch (_type) {
        case GL_TEXTURE_3D:
            glTexParameteri(_type, GL_TEXTURE_WRAP_R, static_cast<GLint>(_wrapping.r));
            [[fallthrough]];
        case GL_TEXTURE_2D:
            glTexParameteri(_type, GL_TEXTURE_WRAP_T, static_cast<GLint>(_wrapping.t));
            [[fallthrough]];
        case GL_TEXTURE_1D:
            glTexParameteri(_type, GL_TEXTURE_WRAP_S, static_cast<GLint>(_wrapping.s));
            break;
        default:
            throw MissingCaseException();
    }
}

void Texture::applySwizzleMask() {
    if (_swizzleMaskChanged) {
        bind();
        glTexParameteriv(_type, GL_TEXTURE_SWIZZLE_RGBA, _swizzleMask.data());
    }
}

void Texture::uploadDataToTexture(void* pixelData) {
    bind();

    switch (_type) {
        case GL_TEXTURE_1D:
            glTexImage1D(
                _type,
                0,
                _internalFormat,
                GLsizei(_dimensions.x),
                0,
                GLenum(_format),
                _dataType,
                pixelData
            );
            break;
        case GL_TEXTURE_2D:
            glTexImage2D(
                _type,
                0,
                _internalFormat,
                GLsizei(_dimensions.x),
                GLsizei(_dimensions.y),
                0,
                GLenum(_format),
                _dataType,
                pixelData
            );
            break;
        case GL_TEXTURE_3D:
            glTexImage3D(
                _type,
                0,
                _internalFormat,
                GLsizei(_dimensions.x),
                GLsizei(_dimensions.y),
                GLsizei(_dimensions.z),
                0,
                GLenum(_format),
                _dataType,
                pixelData
            );
            break;
        default:
            throw MissingCaseException();
    }
}

void Texture::reUploadDataToTexture(void* pixelData) {
    bind();

    switch (_type) {
        case GL_TEXTURE_1D:
            glTexSubImage1D(
                _type,
                0,
                0,
                GLsizei(_dimensions.x),
                GLenum(_format),
                _dataType,
                pixelData
            );
            break;
        case GL_TEXTURE_2D:
            glTexSubImage2D(
                _type,
                0,
                0,
                0,
                GLsizei(_dimensions.x),
                GLsizei(_dimensions.y),
                GLenum(_format),
                _dataType,
                pixelData
            );
            break;
        case GL_TEXTURE_3D:
            glTexSubImage3D(
                _type,
                0,
                0,
                0,
                0,
                GLsizei(_dimensions.x),
                GLsizei(_dimensions.y),
                GLsizei(_dimensions.z),
                GLenum(_format),
                _dataType,
                pixelData
            );
            break;
        default:
            throw MissingCaseException();
    }
}

void Texture::uploadTexture() {
    uploadDataToTexture(_pixels);
}

void Texture::reUploadTexture() {
    reUploadDataToTexture(_pixels);
}

void Texture::uploadTextureFromPBO(GLuint pbo) {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    uploadDataToTexture(nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void Texture::reUploadTextureFromPBO(GLuint pbo) {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    reUploadDataToTexture(nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void Texture::downloadTexture() {
    bind();
    if (!_pixels) {
        allocateMemory();
    }
    glGetTexImage(_type, 0, GLenum(_format), _dataType, _pixels);
}

vec4 Texture::texelAsFloat(unsigned int x) const {
    ghoul_assert(x < compMul(_dimensions), "x must be inside the texture dimensions");
    ghoul_assert(_type == GL_TEXTURE_1D, "Function must be called on a 1D texture");

    if (!_pixels) {
        return vec4(0.f);
    }

    switch (_format) {
        case Format::Red:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    uint8 t = texel<uint8>(x);
                    return {
                        static_cast<float>(t) / numeric_limits<uint8>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_BYTE:
                {
                    int8 t = texel<int8>(x);
                    return {
                        static_cast<float>(t) / numeric_limits<int8>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    uint16 t = texel<uint16>(x);
                    return {
                        static_cast<float>(t) / numeric_limits<uint16>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_SHORT:
                {
                    int16 t = texel<int16>(x);
                    return {
                        static_cast<float>(t) / numeric_limits<int16>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    uint32 t = texel<uint32>(x);
                    return {
                        static_cast<float>(t) / numeric_limits<uint32>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_INT:
                {
                    int32 t = texel<int32>(x);
                    return {
                        static_cast<float>(t) / numeric_limits<int32>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_FLOAT:
                    return { texel<float>(x), 0.f, 0.f, 1.f };
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::RG:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    const tvec2<uint8>& t = texel<tvec2<uint8>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint8>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_BYTE:
                {
                    const tvec2<int8>& t = texel<tvec2<int8>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int8>::max(),
                        static_cast<float>(t.g) / numeric_limits<int8>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    const tvec2<uint16>& t = texel<tvec2<uint16>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint16>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_SHORT:
                {
                    const tvec2<int16>& t = texel<tvec2<int16>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int16>::max(),
                        static_cast<float>(t.g) / numeric_limits<int16>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    const tvec2<uint32>& t = texel<tvec2<uint32>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint32>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_INT:
                {
                    const tvec2<int32>& t = texel<tvec2<int32>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int32>::max(),
                        static_cast<float>(t.g) / numeric_limits<int32>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_FLOAT:
                {
                    const vec2& t = texel<vec2>(x);
                    return { t.r, t.g, 0.f, 1.f };
                }
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::RGB:
            [[fallthrough]];
        case Format::BGR:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    const tvec3<uint8>& t = texel<tvec3<uint8>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint8>::max(),
                        1.f
                    };
                }
                case GL_BYTE:
                {
                    const tvec3<int8>& t = texel<tvec3<int8>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int8>::max(),
                        static_cast<float>(t.g) / numeric_limits<int8>::max(),
                        static_cast<float>(t.b) / numeric_limits<int8>::max(),
                        1.f
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    const tvec3<uint16>& t = texel<tvec3<uint16>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint16>::max(),
                        1.f
                    };
                }
                case GL_SHORT:
                {
                    const tvec3<int16>& t = texel<tvec3<int16>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int16>::max(),
                        static_cast<float>(t.g) / numeric_limits<int16>::max(),
                        static_cast<float>(t.b) / numeric_limits<int16>::max(),
                        1.f
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    const tvec3<uint32>& t = texel<tvec3<uint32>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint32>::max(),
                        1.f
                    };
                }
                case GL_INT:
                {
                    const tvec3<int32>& t = texel<tvec3<int32>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int32>::max(),
                        static_cast<float>(t.g) / numeric_limits<int32>::max(),
                        static_cast<float>(t.b) / numeric_limits<int32>::max(),
                        1.f
                    };
                }
                case GL_FLOAT:
                {
                    const vec3& t = texel<vec3>(x);
                    return { t.r, t.g, t.b, 1.f };
                }
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::RGBA:
            [[fallthrough]];
        case Format::BGRA:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    const tvec4<uint8>& t = texel<tvec4<uint8>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.a) / numeric_limits<uint8>::max()
                    };
                }
                case GL_BYTE:
                {
                    const tvec4<int8>& t = texel<tvec4<int8>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int8>::max(),
                        static_cast<float>(t.g) / numeric_limits<int8>::max(),
                        static_cast<float>(t.b) / numeric_limits<int8>::max(),
                        static_cast<float>(t.a) / numeric_limits<int8>::max()
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    const tvec4<uint16>& t = texel<tvec4<uint16>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.a) / numeric_limits<uint16>::max()
                    };
                }
                case GL_SHORT:
                {
                    const tvec4<int16>& t = texel<tvec4<int16>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int16>::max(),
                        static_cast<float>(t.g) / numeric_limits<int16>::max(),
                        static_cast<float>(t.b) / numeric_limits<int16>::max(),
                        static_cast<float>(t.a) / numeric_limits<int16>::max()
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    const tvec4<uint32>& t = texel<tvec4<uint32>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.a) / numeric_limits<uint32>::max()
                    };
                }
                case GL_INT:
                {
                    const tvec4<int32>& t = texel<tvec4<int32>>(x);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int32>::max(),
                        static_cast<float>(t.g) / numeric_limits<int32>::max(),
                        static_cast<float>(t.b) / numeric_limits<int32>::max(),
                        static_cast<float>(t.a) / numeric_limits<int32>::max()
                    };
                }
                case GL_FLOAT:
                    return texel<vec4>(x);
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::DepthComponent:
            return vec4(0.f);
        default:
            ghoul_assert(false, "Missing case label");
            throw MissingCaseException();
    }
}

vec4 Texture::texelAsFloat(unsigned int x, unsigned int y) const {
    ghoul_assert(
        (y * _dimensions.x) + x < glm::compMul(_dimensions),
        "x and y must be inside the texture dimensions"
    );
    ghoul_assert(_type == GL_TEXTURE_2D, "Function must be called on a 2D texture");

    if (!_pixels) {
        return vec4(0.f);
    }

    switch (_format) {
        case Format::Red:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    uint8 t = texel<uint8>(x, y);
                    return {
                        static_cast<float>(t) / numeric_limits<uint8>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_BYTE:
                {
                    int8 t = texel<int8>(x, y);
                    return {
                        static_cast<float>(t) / numeric_limits<int8>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    uint16 t = texel<uint16>(x, y);
                    return {
                        static_cast<float>(t) / numeric_limits<uint16>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_SHORT:
                {
                    int16 t = texel<int16>(x, y);
                    return {
                        static_cast<float>(t) / numeric_limits<int16>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    uint32 t = texel<uint32>(x, y);
                    return {
                        static_cast<float>(t) / numeric_limits<uint32>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_INT:
                {
                    int32 t = texel<int32>(x, y);
                    return {
                        static_cast<float>(t) / numeric_limits<int32>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_FLOAT:
                {
                    float t = texel<float>(x, y);
                    return { t, 0.f, 0.f, 1.f };
                }
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::RG:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    const tvec2<uint8>& t = texel<tvec2<uint8>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint8>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_BYTE:
                {
                    const tvec2<int8>& t = texel<tvec2<int8>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int8>::max(),
                        static_cast<float>(t.g) / numeric_limits<int8>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    const tvec2<uint16>& t = texel<tvec2<uint16>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint16>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_SHORT:
                {
                    const tvec2<int16>& t = texel<tvec2<int16>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int16>::max(),
                        static_cast<float>(t.g) / numeric_limits<int16>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    const tvec2<uint32>& t = texel<tvec2<uint32>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint32>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_INT:
                {
                    const tvec2<int32>& t = texel<tvec2<int32>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int32>::max(),
                        static_cast<float>(t.g) / numeric_limits<int32>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_FLOAT:
                {
                    const vec2& t = texel<vec2>(x, y);
                    return { t.r, t.g, 0.f, 1.f };
                }
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::RGB: // Intentional fallthrough
        case Format::BGR:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    const tvec3<uint8>& t = texel<tvec3<uint8>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint8>::max(),
                        1.f
                    };
                }
                case GL_BYTE:
                {
                    const tvec3<int8>& t = texel<tvec3<int8>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int8>::max(),
                        static_cast<float>(t.g) / numeric_limits<int8>::max(),
                        static_cast<float>(t.b) / numeric_limits<int8>::max(),
                        1.f
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    const tvec3<uint16>& t = texel<tvec3<uint16>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint16>::max(),
                        1.f
                    };
                }
                case GL_SHORT:
                {
                    const tvec3<int16>& t = texel<tvec3<int16>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int16>::max(),
                        static_cast<float>(t.g) / numeric_limits<int16>::max(),
                        static_cast<float>(t.b) / numeric_limits<int16>::max(),
                        1.f
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    const tvec3<uint32>& t = texel<tvec3<uint32>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint32>::max(),
                        1.f
                    };
                }
                case GL_INT:
                {
                    const tvec3<int32>& t = texel<tvec3<int32>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int32>::max(),
                        static_cast<float>(t.g) / numeric_limits<int32>::max(),
                        static_cast<float>(t.b) / numeric_limits<int32>::max(),
                        1.f
                    };
                }
                case GL_FLOAT:
                {
                    const vec3& t = texel<vec3>(x, y);
                    return { t.r, t.g, t.b, 1.f };
                }
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::RGBA:
            [[fallthrough]];
        case Format::BGRA:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    const tvec4<uint8>& t = texel<tvec4<uint8>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.a) / numeric_limits<uint8>::max()
                    };
                }
                case GL_BYTE:
                {
                    const tvec4<int8>& t = texel<tvec4<int8>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int8>::max(),
                        static_cast<float>(t.g) / numeric_limits<int8>::max(),
                        static_cast<float>(t.b) / numeric_limits<int8>::max(),
                        static_cast<float>(t.a) / numeric_limits<int8>::max()
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    const tvec4<uint16>& t = texel<tvec4<uint16>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.a) / numeric_limits<uint16>::max()
                    };
                }
                case GL_SHORT:
                {
                    const tvec4<int16>& t = texel<tvec4<int16>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int16>::max(),
                        static_cast<float>(t.g) / numeric_limits<int16>::max(),
                        static_cast<float>(t.b) / numeric_limits<int16>::max(),
                        static_cast<float>(t.a) / numeric_limits<int16>::max()
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    const tvec4<uint32>& t = texel<tvec4<uint32>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.a) / numeric_limits<uint32>::max()
                    };
                }
                case GL_INT:
                {
                    const tvec4<int32>& t = texel<tvec4<int32>>(x, y);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int32>::max(),
                        static_cast<float>(t.g) / numeric_limits<int32>::max(),
                        static_cast<float>(t.b) / numeric_limits<int32>::max(),
                        static_cast<float>(t.a) / numeric_limits<int32>::max()
                    };
                }
                case GL_FLOAT:
                    return texel<vec4>(x, y);
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::DepthComponent:
            return vec4(0.f);
        default:
            ghoul_assert(false, "Missing case label");
            throw MissingCaseException();
    }
}

vec4 Texture::texelAsFloat(unsigned int x, unsigned int y, unsigned int z) const {
    ghoul_assert(
        (z * _dimensions.x * _dimensions.y) + (y * _dimensions.x) + x
        < glm::compMul(_dimensions),
        "x, y, and z must be inside the texture dimensions"
    );
    ghoul_assert(_type == GL_TEXTURE_3D, "Function must be called on a 3D texture");

    if (!_pixels) {
        return vec4(0.f);
    }

    switch (_format) {
        case Format::Red:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    uint8 t = texel<uint8>(x, y, z);
                    return {
                        static_cast<float>(t) / numeric_limits<uint8>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_BYTE:
                {
                    int8 t = texel<int8>(x, y, z);
                    return {
                        static_cast<float>(t) / numeric_limits<int8>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    uint16 t = texel<uint16>(x, y, z);
                    return {
                        static_cast<float>(t) / numeric_limits<uint16>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_SHORT:
                {
                    int16 t = texel<int16>(x, y, z);
                    return {
                        static_cast<float>(t) / numeric_limits<int16>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    uint32 t = texel<uint32>(x, y, z);
                    return {
                        static_cast<float>(t) / numeric_limits<uint32>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_INT:
                {
                    int32 t = texel<int32>(x, y, z);
                    return {
                        static_cast<float>(t) / numeric_limits<int32>::max(),
                        0.f,
                        0.f,
                        1.f
                    };
                }
                case GL_FLOAT:
                {
                    float t = texel<float>(x, y, z);
                    return { t, 0.f, 0.f, 1.f };
                }
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::RG:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    const tvec2<uint8>& t = texel<tvec2<uint8>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint8>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_BYTE:
                {
                    const tvec2<int8>& t = texel<tvec2<int8>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int8>::max(),
                        static_cast<float>(t.g) / numeric_limits<int8>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    const tvec2<uint16>& t = texel<tvec2<uint16>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint16>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_SHORT:
                {
                    const tvec2<int16>& t = texel<tvec2<int16>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int16>::max(),
                        static_cast<float>(t.g) / numeric_limits<int16>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    const tvec2<uint32>& t = texel<tvec2<uint32>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint32>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_INT:
                {
                    const tvec2<int32>& t = texel<tvec2<int32>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int32>::max(),
                        static_cast<float>(t.g) / numeric_limits<int32>::max(),
                        0.f,
                        1.f
                    };
                }
                case GL_FLOAT:
                {
                    const vec2& t = texel<vec2>(x, y, z);
                    return { t.r, t.g, 0.f, 1.f };
                }
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::RGB:
            [[fallthrough]];
        case Format::BGR:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    const tvec3<uint8>& t = texel<tvec3<uint8>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint8>::max(),
                        1.f
                    };
                }
                case GL_BYTE:
                {
                    const tvec3<int8>& t = texel<tvec3<int8>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int8>::max(),
                        static_cast<float>(t.g) / numeric_limits<int8>::max(),
                        static_cast<float>(t.b) / numeric_limits<int8>::max(),
                        1.f
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    const tvec3<uint16>& t = texel<tvec3<uint16>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint16>::max(),
                        1.f
                    };
                }
                case GL_SHORT:
                {
                    const tvec3<int16>& t = texel<tvec3<int16>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int16>::max(),
                        static_cast<float>(t.g) / numeric_limits<int16>::max(),
                        static_cast<float>(t.b) / numeric_limits<int16>::max(),
                        1.f
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    const tvec3<uint32>& t = texel<tvec3<uint32>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint32>::max(),
                        1.f
                    };
                }
                case GL_INT:
                {
                    const tvec3<int32>& t = texel<tvec3<int32>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int32>::max(),
                        static_cast<float>(t.g) / numeric_limits<int32>::max(),
                        static_cast<float>(t.b) / numeric_limits<int32>::max(),
                        1.f
                    };
                }
                case GL_FLOAT:
                {
                    const vec3& t = texel<vec3>(x, y, z);
                    return { t.r, t.g, t.b, 1.f };
                }
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::RGBA:
            [[fallthrough]];
        case Format::BGRA:
            switch (_dataType) {
                case GL_UNSIGNED_BYTE:
                {
                    const tvec4<uint8>& t = texel<tvec4<uint8>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint8>::max(),
                        static_cast<float>(t.a) / numeric_limits<uint8>::max()
                    };
                }
                case GL_BYTE:
                {
                    const tvec4<int8>& t = texel<tvec4<int8>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int8>::max(),
                        static_cast<float>(t.g) / numeric_limits<int8>::max(),
                        static_cast<float>(t.b) / numeric_limits<int8>::max(),
                        static_cast<float>(t.a) / numeric_limits<int8>::max()
                    };
                }
                case GL_UNSIGNED_SHORT:
                {
                    const tvec4<uint16>& t = texel<tvec4<uint16>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint16>::max(),
                        static_cast<float>(t.a) / numeric_limits<uint16>::max()
                    };
                }
                case GL_SHORT:
                {
                    const tvec4<int16>& t = texel<tvec4<int16>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int16>::max(),
                        static_cast<float>(t.g) / numeric_limits<int16>::max(),
                        static_cast<float>(t.b) / numeric_limits<int16>::max(),
                        static_cast<float>(t.a) / numeric_limits<int16>::max()
                    };
                }
                case GL_UNSIGNED_INT:
                {
                    const tvec4<uint32>& t = texel<tvec4<uint32>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.g) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.b) / numeric_limits<uint32>::max(),
                        static_cast<float>(t.a) / numeric_limits<uint32>::max()
                    };
                }
                case GL_INT:
                {
                    const tvec4<int32>& t = texel<tvec4<int32>>(x, y, z);
                    return {
                        static_cast<float>(t.r) / numeric_limits<int32>::max(),
                        static_cast<float>(t.g) / numeric_limits<int32>::max(),
                        static_cast<float>(t.b) / numeric_limits<int32>::max(),
                        static_cast<float>(t.a) / numeric_limits<int32>::max()
                    };
                }
                case GL_FLOAT:
                    return texel<vec4>(x, y, z);
                default:
                    ghoul_assert(false, "Missing case label");
                    throw MissingCaseException();
            }
        case Format::DepthComponent:
            return vec4(0.f);
        default:
            ghoul_assert(false, "Missing case label");
            throw MissingCaseException();
    }
}

vec4 Texture::texelAsFloat(const glm::uvec2& pos) const {
    return texelAsFloat(pos.x, pos.y);
}

vec4 Texture::texelAsFloat(const glm::uvec3& pos) const {
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
            ghoul_assert(false, "Missing case label");
            throw MissingCaseException();
    }

    _bpp = static_cast<GLubyte>(szType * numChannels);
}

} // namespace ghoul::opengl
