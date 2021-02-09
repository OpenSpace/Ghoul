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

#ifndef __GHOUL___MODELGEOMETRY___H__
#define __GHOUL___MODELGEOMETRY___H__

#include <ghoul/io/model/modelmesh.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <memory>

namespace ghoul::opengl { class ProgramObject; }

namespace ghoul::modelgeometry {

class ModelGeometry {
public:
    /// The exception that gets thrown if there was an error loading the cache file or
    /// saving this model to a cache file
    struct ModelCacheException : public RuntimeError {
        explicit ModelCacheException(std::string file, std::string msg);

        /// The file that caused the exception to be thrown
        const std::string filename;

        /// The error message that occurred
        const std::string message;
    };

    struct TextureEntry {
        std::string name;
        std::unique_ptr<opengl::Texture> texture;
    };

    ModelGeometry(std::vector<io::ModelMesh> meshes,
        std::vector<TextureEntry> textureStorage);
    ModelGeometry(ModelGeometry&&) noexcept = default;
    ~ModelGeometry() noexcept = default;

    static std::unique_ptr<modelgeometry::ModelGeometry> loadCacheFile(
        const std::string& cachedFile);
    bool saveToCacheFile(const std::string& cachedFile) const;

    void initialize();
    void deinitialize();
    void render(opengl::ProgramObject& program, bool isTexturedModel = true) const;

    double boundingRadius() const;
    void calculateBoundingRadius();

    std::vector<io::ModelMesh>& meshes();
    const std::vector<io::ModelMesh>& meshes() const;
    std::vector<TextureEntry>& textureStorage();
    const std::vector<TextureEntry>& textureStorage() const;

protected:
    double _boundingRadius = 0.0;
    std::vector<io::ModelMesh> _meshes;
    std::vector<TextureEntry> _textureStorage;
};

}  // namespace ghoul::modelgeometry

#endif // __GHOUL___MODELGEOMETRY___H__
