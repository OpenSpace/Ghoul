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

#ifndef __GHOUL___MODELGEOMETRY___H__
#define __GHOUL___MODELGEOMETRY___H__

#include <ghoul/io/model/modelmesh.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <memory>

namespace ghoul::opengl { class ProgramObject; }

namespace ghoul::modelgeometry {

class ModelGeometry {
public:
    ModelGeometry(std::vector<ghoul::io::ModelMesh>&& meshes);
    ModelGeometry(ModelGeometry&&) noexcept = default;
    ~ModelGeometry() noexcept = default;

    bool initialize(float& maximumDistanceSquared);
    void deinitialize();
    void render(ghoul::opengl::ProgramObject& program) const;

    void changeRenderMode(const GLenum mode);
    double boundingRadius() const;
    void calculateBoundingRadius();
    std::vector<ghoul::io::ModelMesh>& meshes();
    const std::vector<ghoul::io::ModelMesh>& meshes() const;
    void setUniforms(ghoul::opengl::ProgramObject& program);

protected:
    double _boundingRadius = 0.0;
    std::vector<ghoul::io::ModelMesh> _meshes;
};

}  // namespace ghoul::modelgeometry

#endif // __GHOUL___MODELGEOMETRY___H__
