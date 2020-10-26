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

#include <ghoul/io/model/modelreaderassimp.h>

#include <ghoul/io/model/modelgeometry.h>
#include <ghoul/io/model/modelmesh.h>
#include <ghoul/io/texture/texturereader.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/fmt.h>
#include <ghoul/glm.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <memory>
#include <vector>

namespace ghoul::io {

void loadMaterialTextures(const aiScene* scene, aiMaterial* mat, aiTextureType type,
                          std::string textureType,
                          std::vector<ghoul::io::ModelMesh::Texture>& textureArray)
{
    ghoul::io::ModelMesh::Texture textureTmp;
    textureTmp.type = textureType;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString pathString;
        mat->GetTexture(type, i, &pathString);

        // Check if texture has already been loaded
        bool shouldSkip = false;
        for (unsigned int j = 0; j < textureArray.size(); ++j) {
            if (textureArray[j].hasTexture) {
                int isLoaded = std::strcmp(textureArray[j].texture->name().c_str(),
                    pathString.C_Str()
                );
                if (isLoaded == 0) {
                    // Texture has already been loaded, continue to next one
                    shouldSkip = true;
                    break;
                }
            }
        }

        // Texture already loaded, do not load it again
        if (shouldSkip)
            continue;

        // Load texture
        // Check if the texture is an embedded texture and whether it is compressed or not
        const aiTexture* texture = scene->GetEmbeddedTexture(pathString.C_Str());
        if (texture) {
            // Embedded texture
            if ((texture->mHeight == 0)) {
                // Load compressed embedded texture
                textureTmp.texture = ghoul::io::TextureReader::ref().loadTexture(
                    static_cast<void*>(texture->pcData),
                    texture->mWidth, texture->achFormatHint
                );
                LDEBUGC(
                    "RenderableModel",
                    fmt::format("Loaded texture from '{}'", static_cast<void*>(texture->pcData))
                );
            }
            else {
                // Load uncompressed embedded texture
                // TODO
            }
        }
        else {
            // Local texture
            textureTmp.texture = ghoul::io::TextureReader::ref().loadTexture(
                absPath(pathString.C_Str())
            );
            LDEBUGC(
                "RenderableModel",
                fmt::format("Loaded texture from '{}'", absPath(pathString.C_Str()))
            );
        }

        // Check if the entire texture is transparent
        bool isOpague = false;
        for (unsigned int j = 0; j < textureTmp.texture->dimensions().x; ++j) {
            if (isOpague)
                break;

            for (unsigned int k = 0; k < textureTmp.texture->dimensions().y; ++k) {
                float alpha = textureTmp.texture->texelAsFloat(glm::vec2(j, k)).a;
                if (alpha != 0) {
                    isOpague = true;
                    break;
                }
            }
        }

        // If entire texture is transparent, do not add it
        if (!isOpague)
            return;

        // Add Texture to the textureArray
        textureTmp.texture->setName(pathString.C_Str());
        textureTmp.hasTexture = true;
        textureArray.push_back(std::move(textureTmp));
    }
}


ModelMesh processMesh(aiMesh* mesh, const aiScene* scene, glm::mat4x4& transform)
{
    std::vector<ModelMesh::Vertex> vertexArray;
    std::vector<unsigned int> indexArray;
    std::vector<ghoul::io::ModelMesh::Texture> textureArray;

    // Go through each of the mesh's vertices
    vertexArray.reserve(mesh->mNumVertices);
    ModelMesh::Vertex vertex;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

        // Positions
        vertex.location[0] = mesh->mVertices[i].x;
        vertex.location[1] = mesh->mVertices[i].y;
        vertex.location[2] = mesh->mVertices[i].z;
        vertex.location[3] = 1.0;

        // Apply the transform to the vertex
        glm::vec4 transformTmp(vertex.location[0],
            vertex.location[1],
            vertex.location[2],
            vertex.location[3]
        );
        transformTmp = transform * transformTmp;
        vertex.location[0] = transformTmp.x;
        vertex.location[1] = transformTmp.y;
        vertex.location[2] = transformTmp.z;
        vertex.location[3] = transformTmp.w;

        // Normals
        if (mesh->HasNormals()) {
            vertex.normal[0] = mesh->mNormals[i].x;
            vertex.normal[1] = mesh->mNormals[i].y;
            vertex.normal[2] = mesh->mNormals[i].z;
        }
        else {
            vertex.normal[0] = 0.f;
            vertex.normal[1] = 0.f;
            vertex.normal[2] = 0.f;
        }

        // Texture Coordinates
        if (mesh->HasTextureCoords(0)) {
            // Each vertex can have at most 8 different texture coordinates.
            // We are using only the first one provided.
            vertex.tex[0] = mesh->mTextureCoords[0][i].x;
            vertex.tex[1] = mesh->mTextureCoords[0][i].y;

            // Tangent, used for normal mapping
            vertex.tangent[0] = mesh->mTangents[i].x;
            vertex.tangent[1] = mesh->mTangents[i].y;
            vertex.tangent[2] = mesh->mTangents[i].z;

            // Bitangent is calculated in shader
        }
        else {
            vertex.tex[0] = 0.f;
            vertex.tex[1] = 0.f;
        }

        vertexArray.push_back(vertex);
    }

    // Walk through the mesh faces and get the vertexes indices
    // Reserve space, every face has usually three indices
    unsigned int nIndices = (mesh->mNumFaces) * static_cast<unsigned int>(3);
    indexArray.reserve(nIndices);
    aiFace face;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indexArray.push_back(face.mIndices[j]);
        }
    }

    // Process materials and textures
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    aiColor3D color(0.f, 0.f, 0.f);
    // We assume a convention for sampler names in the shaders. Each diffuse texture
    // should be named as 'texture_diffuseN' where N is a sequential number ranging
    // from 1 to MAX_SAMPLER_NUMBER. Same applies to other textures as the following
    // list summarizes:
    // diffuse: texture_diffuseN or color_diffuse if embedded simple material instead
    // specular: texture_specularN or color_specular if embedded simple material instead
    // normal: texture_normalN

    // Diffuse
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        loadMaterialTextures(scene,
            material,
            aiTextureType_DIFFUSE,
            "texture_diffuse",
            textureArray
        );
    }
    else {
        // Load embedded simple material instead of textures
        aiReturn hasColor = material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        if (hasColor == AI_SUCCESS) {
            ghoul::io::ModelMesh::Texture textureTmp;
            textureTmp.hasTexture = false;
            textureTmp.type = "color_diffuse";
            textureTmp.color.x = color.r;
            textureTmp.color.y = color.g;
            textureTmp.color.z = color.b;
            textureArray.push_back(std::move(textureTmp));
        }
    }

    // Specular
    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        loadMaterialTextures(scene,
            material,
            aiTextureType_SPECULAR,
            "texture_specular",
            textureArray
        );
    }
    else {
        // Load embedded simple material instead of textures
        color = aiColor3D(0.f, 0.f, 0.f);
        aiReturn hasColor = material->Get(AI_MATKEY_COLOR_SPECULAR, color);
        if (hasColor == AI_SUCCESS && !color.IsBlack()) {
            ghoul::io::ModelMesh::Texture textureTmp;
            textureTmp.hasTexture = false;
            textureTmp.type = "color_specular";
            textureTmp.color.x = color.r;
            textureTmp.color.y = color.g;
            textureTmp.color.z = color.b;
            textureArray.push_back(std::move(textureTmp));
        }
    }

    // Normal
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
        loadMaterialTextures(scene,
            material,
            aiTextureType_NORMALS,
            "texture_normal",
            textureArray
        );
    }

    textureArray.shrink_to_fit();
    return ModelMesh(std::move(vertexArray),
        std::move(indexArray),
        std::move(textureArray)
    );
}

// Process a node in a recursive fashion. Process each individual mesh located 
// at the node and repeats this process on its children nodes (if any)
void processNode(aiNode* node, const aiScene* scene, std::vector<ModelMesh>& meshes,
                 const glm::mat4x4& parentTransform, const bool forceRenderInvisible,
                 const bool notifyInvisibleDropped)
{
    // Convert transform matrix of the node
    // Assimp stores matrixes in row major and glm stores matrixes in column major
    glm::mat4x4 nodeTransform(
        node->mTransformation.a1, node->mTransformation.b1,
        node->mTransformation.c1, node->mTransformation.d1,

        node->mTransformation.a2, node->mTransformation.b2,
        node->mTransformation.c2, node->mTransformation.d2,

        node->mTransformation.a3, node->mTransformation.b3,
        node->mTransformation.c3, node->mTransformation.d3,

        node->mTransformation.a4, node->mTransformation.b4,
        node->mTransformation.c4, node->mTransformation.d4
    );

    glm::mat4x4 globalTransform = parentTransform * nodeTransform;

    // Process each mesh for the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // The node object only contains indices to index the actual objects in the scene. 
        // The scene contains all the data, node is just to keep stuff organized 
        // (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ModelMesh loadedMesh = processMesh(mesh, scene, globalTransform);

        // If mesh is invisible (no materials) drop it unless forced to render anyway
        // Notify unless suppresed
        if (loadedMesh._textures.empty()) {
            // Force invisible mesh to render with flashy colors
            if (forceRenderInvisible) {
                ghoul::io::ModelMesh::Texture textureTmp;
                textureTmp.hasTexture = false;
                textureTmp.useForcedColor = true;
                loadedMesh._textures.push_back(std::move(textureTmp));
            }
            // If not forced to render, drop invisible mesh
            else {
                if (notifyInvisibleDropped) {
                    LINFOC(
                        "RenderableModel",
                        fmt::format("Invisible mesh '{}' dropped", mesh->mName.C_Str())
                    );
                }
                continue;
            }
        }
        meshes.push_back(std::move(loadedMesh));
    }

    // After we've processed all of the meshes (if any) we then recursively 
    // process each of the children nodes (if any)
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i],
            scene,
            meshes,
            globalTransform,
            forceRenderInvisible,
            notifyInvisibleDropped
        );
    }
}

std::unique_ptr<modelgeometry::ModelGeometry> ModelReaderAssimp::loadModel(
                            const std::string& filename, const bool forceRenderInvisible,
                                                 const bool notifyInvisibleDropped) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    // Create an instance of the Importer class
    Assimp::Importer importer;

    // Rendering only triangle meshes.
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw ModelLoadException(filename, importer.GetErrorString(), this);
    }

    // Start with an identity matrix as root transform
    glm::mat4x4 rootTransform;

    // Get info from all models in the scene
    std::vector<ModelMesh> meshArray;
    processNode(scene->mRootNode,
        scene,
        meshArray,
        rootTransform,
        forceRenderInvisible,
        notifyInvisibleDropped
    );

    // Create the ModelGeometry from the meshArray
    modelgeometry::ModelGeometry model(std::move(meshArray));
    return std::make_unique<modelgeometry::ModelGeometry>(std::move(model));
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
