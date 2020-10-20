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

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/io/model/modelgeometry.h>
#include <ghoul/io/model/modelmesh.h>
#include <ghoul/fmt.h>
#include <ghoul/glm.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <memory>
#include <vector>

namespace ghoul::io {

ModelMesh processMesh(aiMesh* mesh, const aiScene* scene, glm::mat4x4& transform) {
    std::vector<ModelMesh::Vertex> vertexArray;
    std::vector<unsigned int> indexArray;

    // Go through each of the mesh's vertices
    vertexArray.reserve(mesh->mNumVertices);
    ModelMesh::Vertex vTmp;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

        // Positions
        vTmp.location[0] = mesh->mVertices[i].x;
        vTmp.location[1] = mesh->mVertices[i].y;
        vTmp.location[2] = mesh->mVertices[i].z;
        vTmp.location[3] = 1.0;

        // Apply the node transform to the vertex
        glm::vec4 transformTmp(vTmp.location[0], vTmp.location[1], vTmp.location[2], 
            vTmp.location[3]);
        transformTmp = transform * transformTmp;
        vTmp.location[0] = transformTmp.x;
        vTmp.location[1] = transformTmp.y;
        vTmp.location[2] = transformTmp.z;
        vTmp.location[3] = transformTmp.w;

        // Normals
        if (mesh->HasNormals()) {
            vTmp.normal[0] = mesh->mNormals[i].x;
            vTmp.normal[1] = mesh->mNormals[i].y;
            vTmp.normal[2] = mesh->mNormals[i].z;
        }
        else {
            vTmp.normal[0] = 0.f;
            vTmp.normal[1] = 0.f;
            vTmp.normal[2] = 0.f;
        }

        // Texture Coordinates
        if (mesh->mTextureCoords[0]) {
            // Each vertex can have at most 8 different texture coordinates.
            // We are using only the first one provided.
            vTmp.tex[0] = mesh->mTextureCoords[0][i].x;
            vTmp.tex[1] = mesh->mTextureCoords[0][i].y;
        }
        else {
            vTmp.tex[0] = 0.f;
            vTmp.tex[1] = 0.f;
        }

        vertexArray.push_back(vTmp);
    }

    // Walk through the mesh faces and get the vertexes indices
    // Reserve space, every face has usually three indices
    unsigned int totalIndices = (mesh->mNumFaces) * static_cast<unsigned int>(3);
    indexArray.reserve(totalIndices);
    aiFace face;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
         face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indexArray.push_back(face.mIndices[j]);
        }
    }
    // Resize in case a face did not have three indices
    indexArray.shrink_to_fit();

    return ModelMesh(std::move(vertexArray), std::move(indexArray));
}

// Process a node in a recursive fashion. Process each individual mesh located 
// at the node and repeats this process on its children nodes (if any)
void processNode(aiNode* node, const aiScene* scene, std::vector<ModelMesh>& meshes) {

    // Process each mesh for the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        
        // Convert transform matrix of the node
        // Assimp stores matrixes in row major and glm stores matrixes in column major
        glm::mat4x4 transform(
            node->mTransformation.a1, node->mTransformation.b1,
            node->mTransformation.c1, node->mTransformation.d1,

            node->mTransformation.a2, node->mTransformation.b2,
            node->mTransformation.c2, node->mTransformation.d2,

            node->mTransformation.a3, node->mTransformation.b3,
            node->mTransformation.c3, node->mTransformation.d3,

            node->mTransformation.a4, node->mTransformation.b4,
            node->mTransformation.c4, node->mTransformation.d4
        );
        
        // The node object only contains indices to index the actual objects in the scene. 
        // The scene contains all the data, node is just to keep stuff organized 
        // (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, transform));
    }
    // After we've processed all of the meshes (if any) we then recursively 
    // process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, meshes);
    }
}

std::unique_ptr<modelgeometry::ModelGeometry> ModelReaderAssimp::loadModel(
                                                        const std::string& filename) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    // Create an instance of the Importer class
    Assimp::Importer importer;

    // Rendering only triangle meshes.
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw ModelLoadException(filename, importer.GetErrorString(), this);
    }

    // Get info from all models in the scene
    std::vector<ModelMesh> meshArray;
    processNode(scene->mRootNode, scene, meshArray);

    // Create the ModelGeometry from the meshArray
    modelgeometry::ModelGeometry model(std::move(meshArray));
    return std::make_unique<modelgeometry::ModelGeometry>(model);
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
