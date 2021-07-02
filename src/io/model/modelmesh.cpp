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

#include <ghoul/io/model/modelmesh.h>

#include <ghoul/io/texture/texturereader.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/opengl/programobject.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/fmt.h>

namespace {
    std::string textureTypeToString(const ghoul::io::ModelMesh::TextureType& type) {
        using TextureType = ghoul::io::ModelMesh::TextureType;
        switch (type) {
            case TextureType::TextureDiffuse:  return "texture_diffuse";
            case TextureType::TextureNormal:   return "texture_normal";
            case TextureType::TextureSpecular: return "texture_specular";
            case TextureType::ColorDiffuse:    return "color_diffuse";
            case TextureType::ColorSpecular:   return "color_specular";
            default:                           throw ghoul::MissingCaseException();
        }
    }
} // namespace

namespace ghoul::io {

ModelMesh::ModelMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
                     std::vector<Texture> textures, bool isInvisible)
    : _vertices(std::move(vertices))
    , _indices(std::move(indices))
    , _textures(std::move(textures))
    , _isInvisible(std::move(isInvisible))
{}

std::string textureTypeToString(const ModelMesh::TextureType& type) {
    switch (type) {
        case ModelMesh::TextureType::TextureDiffuse: return "texture_diffuse";
        case ModelMesh::TextureType::TextureNormal: return "texture_normal";
        case ModelMesh::TextureType::TextureSpecular: return "texture_specular";
        case ModelMesh::TextureType::ColorDiffuse: return "color_diffuse";
        case ModelMesh::TextureType::ColorSpecular: return "color_specular";
        default: throw MissingCaseException();
    }
}

void ModelMesh::generateDebugTexture(ModelMesh::Texture& texture) {
    texture.texture = nullptr;
    texture.hasTexture = false;
    texture.useForcedColor = true;
    texture.type = ModelMesh::TextureType::ColorDiffuse;
}

void ModelMesh::render(opengl::ProgramObject& program, glm::mat4x4 meshTransform,
                       bool isFullyTexturedModel, bool isProjection) const
{
    if (!isProjection) {
        if (isFullyTexturedModel) {
            // Reset shader
            program.setUniform("has_texture_diffuse", false);
            program.setUniform("has_texture_normal", false);
            program.setUniform("has_texture_specular", false);
            program.setUniform("has_color_specular", false);

            // If mesh is invisible and it has not been forced to render then don't render
            if (_isInvisible && _textures.empty()) {
                return;
            }

            // Bind appropriate textures
            int textureCounter = 0;
            for (const Texture& texture : _textures) {
                // Tell shader wether to render invisible mesh with flashy color or not
                program.setUniform("use_forced_color", texture.useForcedColor);
                if (texture.useForcedColor) {
                    break;
                }

                std::string name = textureTypeToString(texture.type);
                // Use texture or color
                if (texture.hasTexture) {
                    // Active proper texture unit before binding
                    glActiveTexture(GL_TEXTURE0 + textureCounter);

                    // Specular special case
                    if (texture.type == TextureType::TextureSpecular) {
                        program.setUniform("has_color_specular", false);
                    }

                    // Tell shader to use textures and set texture unit
                    program.setUniform("has_" + name, true);
                    program.setUniform(name, textureCounter);

                    // And finally bind the texture
                    texture.texture->bind();
                    ++textureCounter;
                }
                // Use embedded simple colors instead of textures
                else {
                    if (texture.type == TextureType::ColorDiffuse) {
                        program.setUniform("has_texture_diffuse", false);
                    }
                    else if (texture.type == TextureType::ColorSpecular) {
                        program.setUniform("has_texture_specular", false);
                        program.setUniform("has_" + name, true);
                    }

                    // Set the color in shader
                    program.setUniform(name, texture.color);
                }
            }
        }
        else {
            // Reset shader
            program.setUniform("has_texture_diffuse", false);

            // Bind appropriate textures
            for (const Texture& texture : _textures) {
                if (texture.type == TextureType::TextureDiffuse ||
                    texture.type == TextureType::ColorDiffuse)
                {
                    // Use texture or color
                    if (texture.hasTexture) {
                        // Active proper texture unit before binding
                        glActiveTexture(GL_TEXTURE1);

                        // Tell shader to use textures and set texture unit
                        program.setUniform("has_texture_diffuse", true);
                        program.setUniform("baseTexture", 1);

                        // And finally bind the texture
                        texture.texture->bind();
                        break;
                    }
                    // Use embedded simple colors instead of textures
                    else {
                        program.setUniform("has_texture_diffuse", false);

                        // Set the color in shader
                        program.setUniform("baseColor", texture.color);
                        break;
                    }
                }
            }
        }
    }

    // Transform mesh
    program.setUniform("meshTransform", meshTransform);
    glm::dmat4 normalTransform = glm::transpose(glm::inverse(meshTransform));
    program.setUniform("meshNormalTransform", glm::mat4(normalTransform));

    // Render the mesh object
    glBindVertexArray(_vaoID);
    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(_indices.size()),
        GL_UNSIGNED_INT,
        nullptr
    );
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

float ModelMesh::calculateBoundingRadius(glm::mat4x4& transform) const {
    // Calculate the bounding sphere of the mesh
    float maximumDistanceSquared = 0.f;
    for (const Vertex& v : _vertices) {
        // Apply the transform to the vertex to get its final position
        glm::vec4 position(v.position[0], v.position[1], v.position[2], 1.f);
        position = transform * position;

        const float d = glm::pow(
            position.x, 2.f) +
            glm::pow(position.y, 2.f) +
            glm::pow(position.z, 2.f
        );

        maximumDistanceSquared = glm::max(d, maximumDistanceSquared);
    }
    return maximumDistanceSquared;
}

void ModelMesh::setInvisible(bool isInvisible) {
    _isInvisible = isInvisible;
}

bool ModelMesh::isInvisible() const {
    return _isInvisible;
}

const std::vector<ModelMesh::Vertex>& ModelMesh::vertices() const {
    return _vertices;
}

const std::vector<unsigned int>& ModelMesh::indices() const {
    return _indices;
}

const std::vector<ModelMesh::Texture>& ModelMesh::textures() const {
    return _textures;
}

void ModelMesh::initialize() {
    if (_vertices.empty()) {
        LERRORC("ModelMesh", "Cannot initialize empty mesh");
        return;
    }

    glGenVertexArrays(1, &_vaoID);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ibo);

    glBindVertexArray(_vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        _vertices.size() * sizeof(Vertex),
        _vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        _indices.size() * sizeof(unsigned int),
        _indices.data(),
        GL_STATIC_DRAW
    );

    // Set vertex attributes pointers
    // Vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

    // Vertex texture coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const GLvoid*>(offsetof(Vertex, tex)) // NOLINT
    );

    // Vertex normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const GLvoid*>(offsetof(Vertex, normal)) // NOLINT
    );

    // Vertex tangent (for normal mapping)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const GLvoid*>(offsetof(Vertex, tangent)) // NOLINT
    );

    glBindVertexArray(0);

    // initialize textures
    // Also chack if there are several textures/colors of the same type for this mesh
    unsigned int nDiffuse = 0;
    unsigned int nSpecular = 0;
    unsigned int nNormal = 0;
    for (const Texture& texture : _textures) {
        switch (texture.type) {
            case TextureType::TextureDiffuse:
            case TextureType::ColorDiffuse:
                ++nDiffuse;
                break;
            case TextureType::TextureSpecular:
            case TextureType::ColorSpecular:
                ++nSpecular;
                break;
            case TextureType::TextureNormal:
                ++nNormal;
                break;
        }

        if (texture.hasTexture) {
            texture.texture->uploadTexture();
            texture.texture->setFilter(opengl::Texture::FilterMode::AnisotropicMipMap);
            texture.texture->purgeFromRAM();
        }
    }

    if (nDiffuse > 1 || nSpecular > 1 || nNormal > 1) {
        LWARNINGC(
            "ModelMesh",
            "More than one texture or color of same type cannot be used for the same "
            "mesh. Only the latest option will be used."
        );
    }
}

void ModelMesh::deinitialize() {
    glDeleteBuffers(1, &_vbo);
    _vbo = 0;
    glDeleteVertexArrays(1, &_vaoID);
    _vaoID = 0;
    glDeleteBuffers(1, &_ibo);
    _ibo = 0;
}

} // namespace ghoul::io
