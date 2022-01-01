/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2022                                                               *
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

#ifndef __GHOUL___MODELREADERLUA___H__
#define __GHOUL___MODELREADERLUA___H__

#include <ghoul/io/model/modelreaderbase.h>

namespace ghoul::io {

/**
 * This model reader reads a model that is specified as a Lua table. The Lua file must
 * return a single top-level table specifying <code>Vertices</code>, <code>Indices</code>,
 * <code>VertexAttribPointers</code>, and the <code>Mode</code>. The <code>Vertices</code>
 * and <code>Indices</code> must be flat table whereas the
 * <code>VertexAttribPointers</code> dictionary must contain tables with the
 * <code>Position</code>, <code>Size</code>, <code>Stride</code>, <code>Offset</code>, and
 * <code>Normalized</code> arguments found in OpenGL's <code>VertexAttribPointer</code>
 * method. The default values for the <code>VertexAttribPointer</code> are:
 * <code>Position = 0</code>, <code>Size = 0</code>, <code>Stride = 0</code>,
 * <code>Offset = 0</code>, and <code>Normalized = false</code>.
 * The value for the <code>type</code> is always <code>GL_FLOAT</code>.<br>
 * The allowed values for the <code>Mode</code> are the strings:
 * <code>GL_LINES</code>, <code>GL_POINTS</code>, <code>GL_LINE_STRIP</code>,
 * <code>GL_LINE_LOOP</code>, <code>GL_LINES</code>, <code>GL_LINE_STRIP_ADJACENCY</code>,
 * <code>GL_LINES_ADJACENCY</code>, <code>GL_TRIANGLE_STRIP</code>,
 * <code>GL_TRIANGLE_FAN</code>, <code>GL_TRIANGLES</code>,
 * <code>GL_TRIANGLE_STRIP_ADJACENCY</code>, <code>GL_TRIANGLES_ADJACENCY</code>, or
 * <code>GL_PATCHES</code>.
 *
 * \sa https://www.opengl.org/sdk/docs/man/html/glVertexAttribPointer.xhtml
 */
class ModelReaderLua : public ModelReaderBase {
public:
    /**
     * Loads the model described as a Lua table and returns the initialized
     * VertexBufferObject.
     *
     * \param filename The Lua file that is to be read. The file must return a single Lua
     *        table
     * \return The initialized VertexBufferObject of the specified model
     *
     * \throw ModelReaderException If there was an exception loading the model
     * \pre \p filename must not be empty
     */
    virtual std::unique_ptr<opengl::VertexBufferObject> loadModel(
        const std::string& filename) const override;
};

} // namespace ghoul::io

#endif // __GHOUL___MODELREADERLUA___H__
