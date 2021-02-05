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

#include <ghoul/io/model/modelgeometry.h>

#include <ghoul/filesystem/file.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/io/texture/texturereader.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/misc/invariants.h>
#include <ghoul/misc/profiling.h>
#include <ghoul/misc/templatefactory.h>
#include <fstream>

namespace ghoul::modelgeometry {

ModelGeometry::ModelGeometry(std::vector<io::ModelMesh> meshes,
                             std::vector<TextureEntry> textureStorage)
    : _meshes(std::move(meshes))
    , _textureStorage(std::move(textureStorage))
{}

double ModelGeometry::boundingRadius() const {
    return _boundingRadius;
}

void ModelGeometry::calculateBoundingRadius() {
    float maximumDistanceSquared = 0.f;

    for (const io::ModelMesh& mesh : _meshes) {
        float d = mesh.calculateBoundingRadius();
        maximumDistanceSquared = std::max(d, maximumDistanceSquared);
    }
    _boundingRadius = maximumDistanceSquared;
}

std::vector<io::ModelMesh>& ModelGeometry::meshes() {
    return _meshes;
}

const std::vector<io::ModelMesh>& ModelGeometry::meshes() const {
    return _meshes;
}

std::vector<ModelGeometry::TextureEntry>& ModelGeometry::textureStorage() {
    return _textureStorage;
}

const std::vector<ModelGeometry::TextureEntry>& ModelGeometry::textureStorage() const {
    return _textureStorage;
}

void ModelGeometry::render(opengl::ProgramObject& program, bool isTexturedModel) const {
    for (const io::ModelMesh& mesh : _meshes) {
        mesh.render(program, isTexturedModel);
    }
}

void ModelGeometry::initialize() {
    ZoneScoped

    for (io::ModelMesh& mesh : _meshes) {
        mesh.initialize();
    }

    calculateBoundingRadius();
}

void ModelGeometry::deinitialize() {
    for (io::ModelMesh& mesh : _meshes) {
        mesh.deinitialize();
    }
}

}  // namespace openspace::modelgeometry
