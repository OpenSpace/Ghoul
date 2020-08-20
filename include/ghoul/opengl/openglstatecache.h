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

#ifndef __GHOUL___OPENGLSTATECACHE___H__
#define __GHOUL___OPENGLSTATECACHE___H__

#include <ghoul/opengl/ghoul_gl.h>

#include <string>
#include <vector>

namespace ghoul::opengl {

/**
 * This class works as cache for the OpenGL most common properties defining the
 * current GL's state.
 * Be aware that not all OpenGL states are available for caching at this moment.
 * New states caching should be added as needed.
 */
class OpenGLStateCache
{
public:
    /* Currently, this class is a Singleton. In the future we will enable multiple
     * instances of the class when working with multiple OpenGL contexts.
     */
    static OpenGLStateCache * getInstance() {
        if (!_singleton) {
            _singleton = new OpenGLStateCache;
            glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &_maxAttachBuffers);
            _singleton->_blendEnabledArray.reserve(_maxAttachBuffers);
        }

        return _singleton;
    }

    OpenGLStateCache(OpenGLStateCache const&) = delete;             // Copy construct
    OpenGLStateCache(OpenGLStateCache&&) = delete;                  // Move construct
    OpenGLStateCache& operator=(OpenGLStateCache const&) = delete;  // Copy assign
    OpenGLStateCache& operator=(OpenGLStateCache &&) = delete;      // Move assign

    bool isCacheInitialized() const { return _cacheInitialized; }
    void loadCurrentGLState();
    void setCachedStates() const;
    void setBlendState() const;
    void setDepthState() const;
    void setLineState() const;
    void setPolygonAndClippingState() const;
    void setViewPortState() const;
    void setViewPortState(const GLint * viewportCoords = nullptr);
    void setColorState() const;
    void setColorState(const GLfloat * clearcolor = nullptr,
                       const GLboolean clampColor = GL_FALSE);

    void viewPort(GLint * viewPort) const;

private:

    /// The logger category that will be used, if a name has been specified.
    std::string _loggerCat = {"OpenGLStateCache"};

    OpenGLStateCache() {};
    ~OpenGLStateCache() = default;

    static OpenGLStateCache* _singleton;
    static GLint _maxAttachBuffers;

    bool _cacheInitialized = false;

    // ViewPort
    GLint _viewport[4] = {0, 0, 0, 0};

    // Polygon and Culling
    GLboolean _faceCullingEnabled = false;
    GLboolean _polygonOffSetEnabled = false;
    GLenum _faceToCull;

    GLfloat _polygonOffSetFactor = 0.f;
    GLfloat _polygonOffSetUnits = 0.f;

    // Color
    GLfloat _colorClearValue[4] = {0.f, 0.f, 0.f, 0.f};
    GLboolean _clampColorEnabled;

    // Depth
    GLboolean _depthTestEnabled = false;
    GLboolean _depthMaskEnabled = true;
    GLfloat _depthClearValue = 1.f;
    GLenum _depthFunction;

    // Blending
    GLboolean _blendEnabled = false;
    std::vector<GLboolean> _blendEnabledArray;
    GLenum _blendEquationRGB;
    GLenum _blendEquationAlpha;
    GLenum _blendDestAlpha;
    GLenum _blendDestRGB;
    GLenum _blendSrcAlpha;
    GLenum _blendSrcRGB;

    // Line
    GLboolean _lineSmoothEnabled = false;
    GLfloat _lineWidth = 1.f;

};

} // namespace ghoul::opengl

#endif // __GHOUL___OPENGLSTATECACHE___H__
