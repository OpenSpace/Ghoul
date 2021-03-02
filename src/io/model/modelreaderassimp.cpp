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

#include <ghoul/io/model/modelreaderassimp.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/io/model/modelgeometry.h>
#include <ghoul/io/model/modelmesh.h>
#include <ghoul/io/texture/texturereader.h>
#include <ghoul/io/texture/texturereaderbase.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/fmt.h>
#include <ghoul/glm.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

namespace {
    constexpr const char* _loggerCat = "ModelReaderAssimp";
} // namespace

namespace ghoul::io {

void generateDebugTexture(ModelMesh::Texture& texture) {
    texture.texture = nullptr;
    texture.hasTexture = false;
    texture.useForcedColor = true;
    texture.type = ModelMesh::TextureType::ColorDiffuse;
}

bool loadMaterialTextures(const aiScene& scene, const aiMaterial& material,
                       const aiTextureType& type, const ModelMesh::TextureType& enumType,
                          std::vector<ModelMesh::Texture>& textureArray,
                 std::vector<modelgeometry::ModelGeometry::TextureEntry>& textureStorage,
                          std::filesystem::path& modelDirectory)
{
    for (unsigned int i = 0; i < material.GetTextureCount(type); ++i) {
        ModelMesh::Texture meshTexture;
        meshTexture.type = enumType;
        aiString path;
        material.GetTexture(type, i, &path);

        // Check for duplicates
        bool shouldSkip = false;
        for (const ModelMesh::Texture& texture : textureArray) {
            if (texture.hasTexture) {
                if (texture.texture->name() == std::string_view(path.C_Str())) {
                    // Texture has already been loaded for this mesh, continue to next one
                    shouldSkip = true;
                    break;
                }
            }
        }

        // Texture already loaded, do not load it again
        if (shouldSkip) {
            continue;
        }

        // Check if texture has already been loaded by other meshes
        for (const modelgeometry::ModelGeometry::TextureEntry& texture : textureStorage) {
            if (texture.texture->name() == std::string_view(path.C_Str())) {
                // Texture has already been loaded. Point to that texture instead of copying
                meshTexture.texture = texture.texture.get();
                meshTexture.texture->setName(path.C_Str());
                meshTexture.hasTexture = true;
                textureArray.push_back(std::move(meshTexture));
                shouldSkip = true;
                break;
            }
        }

        // Texture already loaded, point to it and do not load it again
        if (shouldSkip) {
            continue;
        }

        // Load texture
        const aiTexture* texture = scene.GetEmbeddedTexture(path.C_Str());
        modelgeometry::ModelGeometry::TextureEntry textureEntry;
        textureEntry.name = path.C_Str();
        // Check if the texture is an embedded texture or a local texture
        if (texture) {
            // Embedded texture
            if (texture->mHeight == 0) {
                // Load compressed embedded texture
                try {
                    textureEntry.texture = TextureReader::ref().loadTexture(
                        static_cast<void*>(texture->pcData),
                        texture->mWidth,
                        texture->achFormatHint
                    );
                    meshTexture.texture = textureEntry.texture.get();
                    meshTexture.texture->setName(path.C_Str());
                }
                catch (const TextureReader::InvalidLoadException& e) {
                    LWARNING(fmt::format(
                        "Could not load unsupported texture from '{}' with size"
                        " '{}' : Replacing with flashy color", e._memory, e._size
                    ));
                    generateDebugTexture(meshTexture);
                    textureArray.push_back(std::move(meshTexture));
                    return false;
                }
                catch (const TextureReaderBase::TextureLoadException& e) {
                    LWARNING(fmt::format(
                        "Failed to load texture from '{}' with error: '{}' : "
                        "Replacing with flashy color", e.filename, e.message
                    ));
                    generateDebugTexture(meshTexture);
                    textureArray.push_back(std::move(meshTexture));
                    return false;
                }
            }
            else {
                // Load uncompressed embedded texture
                LWARNING("Uncompressed embedded texture detected: Not supported! "
                    "Replacing with flashy color"
                );
                generateDebugTexture(meshTexture);
                textureArray.push_back(std::move(meshTexture));
                return false;
            }
        }
        else {
            // Local texture
            try {
                std::string pathString(path.C_Str());
                std::string absolutePath =
                    ghoul::filesystem::FileSystem::ref().pathByAppendingComponent(
                        modelDirectory.string(),
                        pathString
                    );

                textureEntry.texture = TextureReader::ref().loadTexture(
                    absPath(absolutePath)
                );
                meshTexture.texture = textureEntry.texture.get();
                meshTexture.texture->setName(path.C_Str());
            }
            catch (const TextureReader::MissingReaderException& e) {
                LWARNING(fmt::format(
                    "Could not load unsupported texture from '{}' with extension"
                    " '{}' : Replacing with flashy color", e.file, e.fileExtension
                ));
                generateDebugTexture(meshTexture);
                textureArray.push_back(std::move(meshTexture));
                return false;
            }
            catch (const TextureReaderBase::TextureLoadException& e) {
                LWARNING(fmt::format(
                    "Failed to load texture from '{}' with error: '{}' : "
                    "Replacing with flashy color", e.filename, e.message
                ));
                generateDebugTexture(meshTexture);
                textureArray.push_back(std::move(meshTexture));
                return false;
            }
        }

        // Check if the entire texture is transparent
        bool isOpaque = false;
        for (unsigned int j = 0; j < meshTexture.texture->dimensions().x; ++j) {
            if (isOpaque) {
                break;
            }

            for (unsigned int k = 0; k < meshTexture.texture->dimensions().y; ++k) {
                float alpha = meshTexture.texture->texelAsFloat(glm::vec2(j, k)).a;
                if (alpha != 0.f) {
                    isOpaque = true;
                    break;
                }
            }
        }

        // If entire texture is transparent, do not add it
        if (!isOpaque) {
            continue;
        }

        // Add new Texture to the textureStorage and point to it in the texture array
        meshTexture.hasTexture = true;
        textureArray.push_back(std::move(meshTexture));
        textureStorage.push_back(std::move(textureEntry));
    }
    return true;
}


ModelMesh processMesh(const aiMesh& mesh, const aiScene& scene,
                      glm::mat4x4& transform,
                 std::vector<modelgeometry::ModelGeometry::TextureEntry>& textureStorage,
                      std::filesystem::path& modelDirectory, bool forceRenderInvisible,
                      bool notifyInvisibleDropped)
{
    std::vector<ModelMesh::Vertex> vertexArray;
    std::vector<unsigned int> indexArray;
    std::vector<ModelMesh::Texture> textureArray;

    // Vertices
    vertexArray.reserve(mesh.mNumVertices);
    for (unsigned int i = 0; i < mesh.mNumVertices; i++) {
        ModelMesh::Vertex vertex;

        // Position
        vertex.position[0] = mesh.mVertices[i].x;
        vertex.position[1] = mesh.mVertices[i].y;
        vertex.position[2] = mesh.mVertices[i].z;

        // Apply the transform to the vertex
        glm::vec4 position(
            vertex.position[0],
            vertex.position[1],
            vertex.position[2],
            1.f
        );
        position = transform * position;
        vertex.position[0] = position.x;
        vertex.position[1] = position.y;
        vertex.position[2] = position.z;

        // Normal
        if (mesh.HasNormals()) {
            vertex.normal[0] = mesh.mNormals[i].x;
            vertex.normal[1] = mesh.mNormals[i].y;
            vertex.normal[2] = mesh.mNormals[i].z;
        }
        else {
            vertex.normal[0] = 0.f;
            vertex.normal[1] = 0.f;
            vertex.normal[2] = 0.f;
        }

        // Texture Coordinates
        if (mesh.HasTextureCoords(0)) {
            // Each vertex can have at most 8 different texture coordinates.
            // We are using only the first one provided.
            vertex.tex[0] = mesh.mTextureCoords[0][i].x;
            vertex.tex[1] = mesh.mTextureCoords[0][i].y;
        }
        else {
            vertex.tex[0] = 0.f;
            vertex.tex[1] = 0.f;
        }

        // Tangent, used for normal mapping
        // Bitangent is calculated in shader
        if (mesh.HasTangentsAndBitangents()) {
            vertex.tangent[0] = mesh.mTangents[i].x;
            vertex.tangent[1] = mesh.mTangents[i].y;
            vertex.tangent[2] = mesh.mTangents[i].z;
        }
        else {
            vertex.tangent[0] = 0.f;
            vertex.tangent[1] = 0.f;
            vertex.tangent[2] = 0.f;
        }

        vertexArray.push_back(std::move(vertex));
    }

    // Indices
    // Reserve space, every face has usually three indices
    unsigned int nIndices = mesh.mNumFaces * 3u;
    indexArray.reserve(nIndices);
    for (unsigned int i = 0; i < mesh.mNumFaces; ++i) {
        aiFace face = mesh.mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indexArray.push_back(face.mIndices[j]);
        }
    }

    // Process materials and textures
    aiMaterial* material = scene.mMaterials[mesh.mMaterialIndex];

    // If there is not material then do not add the mesh
    if (!material) {
        return ModelMesh(
            std::move(vertexArray),
            std::move(indexArray),
            std::move(textureArray)
        );
    }

    // We assume a convention for sampler names in the shaders. Each diffuse texture
    // should be named as 'texture_diffuse', at the moment only one texture per type is
    // supported. Same applies to other textures as the following
    // list summarizes:
    // diffuse: texture_diffuse or color_diffuse if embedded simple material instead
    // specular: texture_specular or color_specular if embedded simple material instead
    // normal: texture_normal

    // Opacity
    float opacity = 0.f;
    aiReturn hasOpacity = material->Get(AI_MATKEY_OPACITY, opacity);
    if (hasOpacity == AI_SUCCESS && opacity == 0.f) {
        // If the material is transparent then do not add it
        return ModelMesh(
            std::move(vertexArray),
            std::move(indexArray),
            std::move(textureArray)
        );
    }

    // Diffuse
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        bool success = loadMaterialTextures(
            scene,
            *material,
            aiTextureType_DIFFUSE,
            ModelMesh::TextureType::TextureDiffuse,
            textureArray,
            textureStorage,
            modelDirectory
        );

        if (!success) {
            return ModelMesh(
                std::move(vertexArray),
                std::move(indexArray),
                std::move(textureArray)
            );
        }
    }
    else {
        // Load embedded simple material instead of textures
        aiColor4D color4(0.f, 0.f, 0.f, 0.f);
        aiReturn hasColor4 = material->Get(AI_MATKEY_COLOR_DIFFUSE, color4);
        if (hasColor4 == AI_SUCCESS) {
            // Only add the color if it is not transparent
            if (color4.a != 0.f) {
                ModelMesh::Texture texture;
                texture.hasTexture = false;
                texture.type = ModelMesh::TextureType::ColorDiffuse;
                texture.color.x = color4.r;
                texture.color.y = color4.g;
                texture.color.z = color4.b;
                textureArray.push_back(std::move(texture));
            }
        }
        else {
            aiColor3D color3(0.f, 0.f, 0.f);
            aiReturn hasColor3 = material->Get(AI_MATKEY_COLOR_DIFFUSE, color3);
            if (hasColor3 == AI_SUCCESS) {
                ModelMesh::Texture texture;
                texture.hasTexture = false;
                texture.type = ModelMesh::TextureType::ColorDiffuse;
                texture.color.x = color3.r;
                texture.color.y = color3.g;
                texture.color.z = color3.b;
                textureArray.push_back(std::move(texture));
            }
        }
    }

    // Specular
    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        bool success = loadMaterialTextures(
            scene,
            *material,
            aiTextureType_SPECULAR,
            ModelMesh::TextureType::TextureSpecular,
            textureArray,
            textureStorage,
            modelDirectory
        );

        if (!success) {
            return ModelMesh(
                std::move(vertexArray),
                std::move(indexArray),
                std::move(textureArray)
            );
        }
    }
    else {
        // Load embedded simple material instead of textures
        aiColor4D color4(0.f, 0.f, 0.f, 0.f);
        aiReturn hasColor4 = material->Get(AI_MATKEY_COLOR_SPECULAR, color4);
        if (hasColor4 == AI_SUCCESS && !color4.IsBlack()) {
            // Only add the color if it is not transparent
            if (color4.a != 0.f) {
                ModelMesh::Texture texture;
                texture.hasTexture = false;
                texture.type = ModelMesh::TextureType::ColorSpecular;
                texture.color.x = color4.r;
                texture.color.y = color4.g;
                texture.color.z = color4.b;
                textureArray.push_back(std::move(texture));
            }
        }
        else {
            aiColor3D color3(0.f, 0.f, 0.f);
            aiReturn hasColor3 = material->Get(AI_MATKEY_COLOR_SPECULAR, color3);
            if (hasColor3 == AI_SUCCESS && !color3.IsBlack()) {
                ModelMesh::Texture texture;
                texture.hasTexture = false;
                texture.type = ModelMesh::TextureType::ColorSpecular;
                texture.color.x = color3.r;
                texture.color.y = color3.g;
                texture.color.z = color3.b;
                textureArray.push_back(std::move(texture));
            }
        }
    }

    // Normal
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
        bool success = loadMaterialTextures(
            scene,
            *material,
            aiTextureType_NORMALS,
            ModelMesh::TextureType::TextureNormal,
            textureArray,
            textureStorage,
            modelDirectory
        );

        if (!success) {
            return ModelMesh(
                std::move(vertexArray),
                std::move(indexArray),
                std::move(textureArray)
            );
        }
    }

    // If mesh is invisible (no materials or textures) drop it unless forced to render
    // Notify unless suppresed
    if (textureArray.empty()) {
        if (forceRenderInvisible) {
            // Force invisible mesh to render with flashy colors
            ModelMesh::Texture texture;
            generateDebugTexture(texture);
            textureArray.push_back(std::move(texture));
        }
        // If not forced to render, drop invisible mesh
        else if (notifyInvisibleDropped) {
            LINFO(fmt::format("Invisible mesh '{}' dropped", mesh.mName.C_Str()));
        }
    }

    textureArray.shrink_to_fit();
    return ModelMesh(
        std::move(vertexArray),
        std::move(indexArray),
        std::move(textureArray)
    );
}

// Process a node in a recursive fashion. Process each individual mesh located 
// at the node and repeats this process on its children nodes (if any)
void processNode(const aiNode& node, const aiScene& scene, std::vector<ModelMesh>& meshes,
                 glm::mat4x4& parentTransform,
                 std::vector<modelgeometry::ModelGeometry::TextureEntry>& textureStorage,
                 bool forceRenderInvisible, bool notifyInvisibleDropped,
                 std::filesystem::path& modelDirectory)
{
    // Convert transform matrix of the node
    // Assimp stores matrixes in row major and glm stores matrixes in column major
    glm::mat4x4 nodeTransform(
        node.mTransformation.a1, node.mTransformation.b1,
        node.mTransformation.c1, node.mTransformation.d1,

        node.mTransformation.a2, node.mTransformation.b2,
        node.mTransformation.c2, node.mTransformation.d2,

        node.mTransformation.a3, node.mTransformation.b3,
        node.mTransformation.c3, node.mTransformation.d3,

        node.mTransformation.a4, node.mTransformation.b4,
        node.mTransformation.c4, node.mTransformation.d4
    );

    glm::mat4x4 globalTransform = parentTransform * nodeTransform;

    // Process each mesh for the current node
    for (unsigned int i = 0; i < node.mNumMeshes; i++) {
        // The node object only contains indices to the actual objects in the scene
        // The scene contains all the data, node is just to keep stuff organized
        // (like relations between nodes)
        aiMesh* mesh = scene.mMeshes[node.mMeshes[i]];
        ModelMesh loadedMesh = processMesh(
            *mesh,
            scene,
            globalTransform,
            textureStorage,
            modelDirectory,
            forceRenderInvisible,
            notifyInvisibleDropped
        );

        // Don't render invisible meshes
        if (loadedMesh.textures().empty()) {
            continue;
        }

        meshes.push_back(std::move(loadedMesh));
    }

    // After we've processed all of the meshes (if any) we then recursively 
    // process each of the children nodes (if any)
    for (unsigned int i = 0; i < node.mNumChildren; i++) {
        processNode(
            *(node.mChildren[i]),
            scene,
            meshes,
            globalTransform,
            textureStorage,
            forceRenderInvisible,
            notifyInvisibleDropped,
            modelDirectory
        );
    }
}


std::unique_ptr<modelgeometry::ModelGeometry> ModelReaderAssimp::loadModel(
                                        std::string& filename, bool forceRenderInvisible,
                                                       bool notifyInvisibleDropped) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    std::filesystem::path modelDirectory = std::filesystem::path(filename).parent_path();

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        filename,
        aiProcess_Triangulate |         // Only triangles
        aiProcess_GenSmoothNormals |    // Generate smooth normals
        aiProcess_CalcTangentSpace      // Generate tangents and bitangents
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw ModelLoadException(filename, importer.GetErrorString(), this);
    }

    // Start with an identity matrix as root transform
    glm::mat4x4 rootTransform;

    // Get info from all models in the scene
    std::vector<ModelMesh> meshArray;
    meshArray.reserve(scene->mNumMeshes);
    std::vector<modelgeometry::ModelGeometry::TextureEntry> textureStorage;
    textureStorage.reserve(scene->mNumTextures);
    processNode(
        *(scene->mRootNode),
        *scene,
        meshArray,
        rootTransform,
        textureStorage,
        forceRenderInvisible,
        notifyInvisibleDropped,
        modelDirectory
    );

    // Return the ModelGeometry from the meshArray
    return std::make_unique<modelgeometry::ModelGeometry>(
        std::move(meshArray),
        std::move(textureStorage)
    );
}

std::vector<std::string> ModelReaderAssimp::supportedExtensions() const {
    // Taken from https://github.com/assimp/assimp
    return {
        "fbx",          // Autodesk
        "dae",          // Collada
        "gltf", "glb",  // glTF
        "blend",        // Blender 3D
        "3ds",          // 3ds Max 3DS
        "ase",          // 3ds Max ASE
        "obj",          // Wavefront Object
        "ifc",          // Industry Foundation Classes(IFC / Step)
        "xgl", "zgl"    // XGL
        "ply",          // Stanford Polygon Library
        "dxf",          // * AutoCAD DXF
        "lwo",          // LightWave
        "lws",          // LightWave Scene
        "lxo",          // Modo
        "stl",          // Stereolithography
        "x",            // DirectX X
        "ac",           // AC3D
        "ms3d",         // Milkshape 3D
        "cob", "scn"    // * TrueSpace
    };
}

} // namespace ghoul::io
