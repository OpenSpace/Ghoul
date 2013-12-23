/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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


#include <ghoul/opengl/textureunit.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/syscapabilities.h>

#include <exception>
#include <stdexcept>

namespace {
    const std::string _loggerCat = "TextureUnit";
}

namespace ghoul {
namespace opengl {

bool TextureUnit::_initialized = false;
unsigned int TextureUnit::_totalActive = 0;
unsigned int TextureUnit::_maxTexUnits = 0;
std::vector<bool> TextureUnit::_busyUnits = std::vector<bool>();

TextureUnit::TextureUnit()
    : _number(0)
    , _glEnum(0)
    , _assigned(false)
{
    if (!_initialized)
        initialize();
}

TextureUnit::~TextureUnit() {
    if (_assigned) {
        _busyUnits.at(_number) = false;
        --_totalActive;
    }
}

void TextureUnit::activate() {
    if (!_assigned)
        assignUnit();
    glActiveTexture(_glEnum);
}

GLint TextureUnit::glEnum() {
    if (!_assigned)
        assignUnit();
    return _glEnum;
}

GLint TextureUnit::unitNumber() {
    if (!_assigned)
        assignUnit();
    return _number;
}

TextureUnit::operator GLint() {
    return unitNumber();
}

void TextureUnit::setZeroUnit() {
    glActiveTexture(GL_TEXTURE0);
}

void TextureUnit::deinitialize() {
    for (auto it = _busyUnits.begin(); it != _busyUnits.end(); ++it)
        (*it) = false;
    _totalActive = 0;
    setZeroUnit();
}

int TextureUnit::numberActiveUnits() {
    return _totalActive;
}

void TextureUnit::assignUnit() {
    if (_totalActive >= _maxTexUnits) {
        LERROR_SAFE("No more texture units available");
        return;
    }

    _assigned = true;

    for (size_t i = 0; i < _maxTexUnits; ++i) {
        if (!_busyUnits[i]) {
            _number = static_cast<GLint>(i);
            _glEnum = GL_TEXTURE0 + _number;
            _busyUnits[i] = true;
            ++_totalActive;
            break;
        }
    }
}

void TextureUnit::initialize() {
    SystemCapabilities::init();
    _maxTexUnits = SysCap.maximumNumberOfTextureUnits();
    _busyUnits = std::vector<bool>(_maxTexUnits, false);
    _initialized = true;
}

} // namespace opengl
} // namespace ghoul
