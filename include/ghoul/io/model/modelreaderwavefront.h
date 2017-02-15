/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#ifndef __GHOUL___MODELREADERWAVEFRONT___H__
#define __GHOUL___MODELREADERWAVEFRONT___H__

#include <ghoul/io/model/modelreaderbase.h>

namespace ghoul {
namespace io {

/**
 * This model reader loads the provided file using the TinyObjLoader library. This simple
 * method loads multiple shapes, but does not work on the materials described in the OBJ
 * file.
 * \sa https://github.com/syoyo/tinyobjloader
 */
class ModelReaderWavefront: public ModelReaderBase {
public:
    /**
     * Loads the OBJ file pointed to by \p filename and returns a constructed
     * VertexBufferObject from it. Provided materials are ignored and all shapes are
     * collapsed into one VertexBufferObject.
     * \param filename The OBJ file to be loaded
     * \return The initialized VertexBufferObject containing the model or models
     * \throw ModelReaderException If there was an error reading the model from
     * \p filename
     * \pre \p filename must not be empty
     */
    std::unique_ptr<opengl::VertexBufferObject> loadModel(
        const std::string& filename) const;
};
    
} // namespace io
} // namespace ghoul

#endif // __GHOUL___MODELREADERWAVEFRONT___H__
