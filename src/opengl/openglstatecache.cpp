/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2020                                                               *
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

#include <ghoul/opengl/openglstatecache.h>

#include <ghoul/fmt.h>
#include <ghoul/logging/logmanager.h>

#include <ghoul/misc/assert.h>

namespace ghoul::opengl {

OpenGLStateCache* OpenGLStateCache::_singleton = nullptr;
GLint OpenGLStateCache::_maxAttachBuffers = 8;

void OpenGLStateCache::loadCurrentGLState() {
    // ViewPort
    glGetIntegerv(GL_VIEWPORT, _viewport);

    // Polygon and Culling
    _faceCullingEnabled = glIsEnabled(GL_CULL_FACE);
    glGetIntegerv(GL_CULL_FACE_MODE, &_faceToCull);
    _polygonOffSetEnabled = glIsEnabled(GL_POLYGON_OFFSET_FILL);
    glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &_polygonOffSetFactor);
    glGetFloatv(GL_POLYGON_OFFSET_UNITS, &_polygonOffSetUnits);

    // Color
    glGetFloatv(GL_COLOR_CLEAR_VALUE, _colorClearValue);
    glGetBooleanv(GL_CLAMP_READ_COLOR, &_clampColorEnabled);

    // Depth
    glGetFloatv(GL_DEPTH_CLEAR_VALUE, &_depthClearValue);
    _depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    _depthMaskEnabled = glIsEnabled(GL_DEPTH_WRITEMASK);
    glGetIntegerv(GL_DEPTH_FUNC, &_depthFunction);

    // Blending
    _blendEnabled = glIsEnabled(GL_BLEND);

    if (_blendEnabledArray.empty()) {
        for (int i = 0; i < _maxAttachBuffers; ++i) {
            _blendEnabledArray.push_back(glIsEnabledi(GL_BLEND, i));
        }
    } else {
        std::vector<GLboolean>::iterator endBlendBuffer = _blendEnabledArray.end();
        std::vector<GLboolean>::iterator blendBufferIt = _blendEnabledArray.begin();
        for (int i = 0; blendBufferIt < endBlendBuffer; ++ blendBufferIt, ++i) {
            *blendBufferIt = glIsEnabledi(GL_BLEND, i);
        }
    }

    glGetIntegerv(GL_BLEND_EQUATION_RGB, &_blendEquationRGB);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &_blendEquationAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &_blendDestAlpha);
    glGetIntegerv(GL_BLEND_DST_RGB, &_blendDestRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &_blendSrcAlpha);
    glGetIntegerv(GL_BLEND_SRC_RGB, &_blendSrcRGB);

    // Line
    _lineSmoothEnabled = glIsEnabled(GL_LINE_SMOOTH);
    glGetFloatv(GL_LINE_WIDTH, &_lineWidth);

    _cacheInitialized = true;
}

void OpenGLStateCache::setCachedStates() const {
    ghoul_assert(!_cacheInitialized, "OpenGL State cache is not initialized.");
    setViewPortState();
    setColorState();
    setBlendState();
    setDepthState();
    setLineState();
    setPolygonAndClippingState();
}

void OpenGLStateCache::setBlendState() const {
    if (_blendEnabled) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }

    std::vector<GLboolean>::const_iterator endBlendBuffer = _blendEnabledArray.cend();
    std::vector<GLboolean>::const_iterator blendBufferIt  = _blendEnabledArray.cbegin();
    for (int i = 0; blendBufferIt < endBlendBuffer; ++ blendBufferIt, ++i) {
        if (*blendBufferIt) {
            glEnablei(GL_BLEND, i);
        } else {
            glDisablei(GL_BLEND, i);
        }
    }

    glBlendEquationSeparate(_blendEquationRGB, _blendEquationAlpha);
    glBlendFuncSeparate(_blendSrcRGB, _blendDestRGB, _blendSrcAlpha, _blendDestAlpha);
}

void OpenGLStateCache::setDepthState() const {
    if (_depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }

    if (_depthMaskEnabled) {
        glDepthMask(false);
    } else {
        glDepthMask(true);
    }

    glClearDepth(_depthClearValue);
    glDepthFunc(_depthFunction);
}

void OpenGLStateCache::setLineState() const {
    if (_lineSmoothEnabled) {
        glEnable(GL_LINE_SMOOTH);
    } else {
        glDisable(GL_LINE_SMOOTH);
    }
    glLineWidth(_lineWidth);
}

void OpenGLStateCache::setPolygonAndClippingState() const {
    if (_faceCullingEnabled) {
        glEnable(GL_CULL_FACE);
    }
    else {
        glDisable(GL_CULL_FACE);
    }

    glCullFace(_faceToCull);

    if (_polygonOffSetEnabled) {
        glEnable(GL_POLYGON_OFFSET_FILL);
    }
    else {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    glPolygonOffset(_polygonOffSetFactor, _polygonOffSetUnits);
}

void OpenGLStateCache::setColorState() const {
    glClearColor(
        _colorClearValue[0],
        _colorClearValue[1],
        _colorClearValue[2],
        _colorClearValue[3]
    );
}

void OpenGLStateCache::setColorState(const GLfloat * color, const GLboolean clampColor)  {
    if (color != nullptr) {
        if (color[0] != _colorClearValue[0] ||
            color[1] != _colorClearValue[1] ||
            color[2] != _colorClearValue[2] ||
            color[3] != _colorClearValue[3])
        {
            _colorClearValue[0] = color[0];
            _colorClearValue[1] = color[1];
            _colorClearValue[2] = color[2];
            _colorClearValue[3] = color[3];
        }

        if (clampColor != _clampColorEnabled) {
            _clampColorEnabled = clampColor;
            glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
        }
    }

    glClearColor(
        _colorClearValue[0],
        _colorClearValue[1],
        _colorClearValue[2],
        _colorClearValue[3]
    );
}

void OpenGLStateCache::setViewPortState() const {
    glViewport(_viewport[0], _viewport[1], _viewport[2], _viewport[3]);
}

void OpenGLStateCache::setViewPortState(const GLint * viewportCoords)  {
    if (viewportCoords != nullptr) {
        if (viewportCoords[0] != _viewport[0] ||
            viewportCoords[1] != _viewport[1] ||
            viewportCoords[2] != _viewport[2] ||
            viewportCoords[3] != _viewport[3])
        {
            _viewport[0] = viewportCoords[0];
            _viewport[1] = viewportCoords[1];
            _viewport[2] = viewportCoords[2];
            _viewport[3] = viewportCoords[3];
        }
    }

    glViewport(_viewport[0], _viewport[1], _viewport[2], _viewport[3]);
}

void OpenGLStateCache::viewPort(GLint * viewPort) const {
    if (viewPort == nullptr) {
        LERROR("Unitialized memory argument passed to viewPort method.");
    } else {
        viewPort[0] = _viewport[0];
        viewPort[1] = _viewport[1];
        viewPort[2] = _viewport[2];
        viewPort[3] = _viewport[3];
    }
}

} // namespace ghoul::opengl
