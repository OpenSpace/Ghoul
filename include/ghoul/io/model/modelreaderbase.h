/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2021                                                               *
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

#include <ghoul/io/model/modelgeometry.h>
#include <ghoul/misc/exception.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <memory>
#include <string>

namespace ghoul::modelgeometry { class ModelGeometry; }

namespace ghoul::io {

/**
 * Concrete instantiations of this abstract base class provide the ability to load
 * geometric models from a file on disk into a ModelGeometry.
 * A valid OpenGL context has to be present for the loadModel function.
 */
class ModelReaderBase {
public:
    /// The exception that gets thrown if there was an error loading the Model
    /// from file or cache file
    struct ModelLoadException : public RuntimeError {
        explicit ModelLoadException(std::string name, std::string msg,
            const ModelReaderBase* r);

        /// The file that caused the exception to be thrown
        const std::string filename;

        /// The error message that occurred
        const std::string message;

        /// The ModelReaderBase that caused the exception
        const ModelReaderBase* reader;
    };

    /// The exception that gets thrown if there was an error saving the Model to cache
    struct ModelSaveException : public RuntimeError {
        explicit ModelSaveException(std::string name, std::string msg,
            const ModelReaderBase* r);

        /// The file that caused the exception to be thrown
        const std::string filename;

        /// The error message that occurred
        const std::string message;

        /// The ModelReaderBase that caused the exception
        const ModelReaderBase* reader;
    };

    /// Default virtual destructor
    virtual ~ModelReaderBase() = default;

    /**
     * The method loading the specific model from disk. The result is a ModelGeometry
     *
     * \param filename The file on disk that is to be loaded
     * \param forceRenderInvisible Force invisible meshes to render or not
     * \param notifyInvisibleDropped Notify in log if invisible meshses were dropped
     * \return The ModelGeometry
     *
     * \throw ModelLoadException If there was an error loading the model from disk
     */
    virtual std::unique_ptr<modelgeometry::ModelGeometry> loadModel(
        const std::string& filename, const bool forceRenderInvisible = false,
        const bool notifyInvisibleDropped = true) const = 0;

    /**
     * Method to load an already cached model from disk. Result is a ModelGeometry
     *
     * \param cachedFile The cache file to be loaded on disk
     * \param forceRenderInvisible Force invisible meshes to render or not
     * \return The ModelGeometry
     *
     * \throw ModelLoadException If there was an error loading the model from cache
     */
    virtual std::unique_ptr<modelgeometry::ModelGeometry> loadCachedFile(
        const std::string& cachedFile) const = 0;

    /**
     * Method to save an already loaded model to a cache. Result is a bool telling if
     * saving was successfull or not
     *
     * \param cachedFile The file the cache should be saved to
     * \param model Pointer to the ModelGeometry that should be saved to the cache file
     * \return bool if successfull or not
     *
     * \throw ModelSaveException If there was an error saving the model to cache
     */
    virtual bool saveCachedFile(const std::string& cachedFile,
        const modelgeometry::ModelGeometry* model) const = 0;

    /**
     * Returns a list of all extensions.
     *
     * \return The supported file extensions.
     */
    virtual std::vector<std::string> supportedExtensions() const = 0;
};

} // namespace ghoul::io

#endif // __GHOUL___MODELREADERBASE___H__
