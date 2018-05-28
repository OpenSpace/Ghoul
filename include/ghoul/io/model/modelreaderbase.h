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
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#ifndef __GHOUL___MODELREADERBASE___H__
#define __GHOUL___MODELREADERBASE___H__

#include <ghoul/misc/exception.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <memory>
#include <string>

namespace ghoul::opengl { class VertexBufferObject; }

namespace ghoul::io {

/**
 * Concrete instantiations of this abstract base class provide the ability to load
 * geometric models from a file on disk into a VertexBufferObject. The resulting
 * VertexBufferObject is fully initialized and usable. A valid OpenGL context has to be
 * present for the loadModel function.
 */
class ModelReaderBase {
public:
    /**
     * Vertex base properties.
     */
    struct Vertex {
        GLfloat location[3];
        GLfloat tex[2];
        GLfloat normal[3];
    };

    /**
     * The exception that gets thrown if there is an error loading a model from the
     * provided \p file. The \p error message is contained in the exception.
     */
    struct ModelReaderException : public RuntimeError {
        explicit ModelReaderException(std::string file, std::string error);

        /// The file which caused the exception
        std::string fileName;

        /// The error message
        std::string errorMessage;
    };

    /// Default virtual destructor
    virtual ~ModelReaderBase() = default;

    /**
     * The method loading the specific model from disk. The result is a fully initialized
     * and usable VertexBufferObject
     *
     * \param filename The file on disk that is to be loaded
     * \return The intialized VertexBufferObject
     *
     * \throw ModelReaderException If there was an error loading the model from disk
     */
    virtual std::unique_ptr<opengl::VertexBufferObject> loadModel(
        const std::string& filename) const = 0;
};

} // namespace ghoul::io

#endif // __GHOUL___MODELREADERBASE___H__
