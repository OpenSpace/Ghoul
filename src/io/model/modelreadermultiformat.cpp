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

#include <ghoul/io/model/modelreadermultiformat.h>

#include <ghoul/fmt.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/opengl/vertexbufferobject.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <memory>
#include <vector>

namespace {
    void getMeshesInScene(size_t& totalSizeIndex, size_t& totalSizeVertex,
        std::vector<const aiMesh*>& meshArray, const aiNode* /*currNode*/,
        const aiScene* scene)
    {
        for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
            meshArray.push_back(scene->mMeshes[i]);
            totalSizeIndex += meshArray.back()->mNumFaces * 3;
            totalSizeVertex += meshArray.back()->mNumVertices;
        }

        /*for (auto m = 0; m < currNode->mNumMeshes; ++m)
        {
        meshArray.push_back(scene->mMeshes[currNode->mMeshes[m]]);

        for (auto nf = 0; nf < meshArray.back()->mNumFaces; ++nf)
        {
        const struct aiFace* face = &meshArray.back()->mFaces[nf];

        *totalSizeIndex += face->mNumIndices;
        }

        *totalSizeVertex += meshArray.back()->mNumVertices;
        }

        for (auto nc = 0; nc < currNode->mNumChildren; ++nc)
        {
        const aiNode * nextNode = currNode->mChildren[nc];
        getMeshesInScene(totalSizeIndex, totalSizeVertex, meshArray, nextNode, scene);
        }*/
    }
} // namespace

namespace ghoul::io {

std::unique_ptr<opengl::VertexBufferObject> ModelReaderMultiFormat::loadModel(
    const std::string& filename) const
{
    std::vector<Vertex> vertices;
    std::vector<int> indices;

    loadModel(filename, vertices, indices);

    std::unique_ptr<opengl::VertexBufferObject> vbo =
        std::make_unique<opengl::VertexBufferObject>();
    vbo->initialize(vertices, indices);
    vbo->vertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        sizeof(Vertex),
        offsetof(Vertex, location) // NOLINT
    );
    vbo->vertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        sizeof(Vertex),
        offsetof(Vertex, tex) // NOLINT
    );
    vbo->vertexAttribPointer(
        2,
        3,
        GL_FLOAT,
        sizeof(Vertex),
        offsetof(Vertex, normal) // NOLINT
    );
    return vbo;
}

void ModelReaderMultiFormat::loadModel(
    const std::string& filename,
    std::vector<Vertex> & vertexArray,
    std::vector<int> & indexArray) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    // Create an instance of the Importer class
    Assimp::Importer importer;

    // Rendering only triangle meshes.
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_GenNormals |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices);

    if (!scene) {
        throw ModelReaderException(
            filename,
            importer.GetErrorString()
        );
    }

    // Get info from all models in the scene
    size_t totalSizeIndex = 0;
    size_t totalSizeVertex = 0;
    std::vector< const struct aiMesh *> meshArray;

    getMeshesInScene(
        totalSizeIndex,
        totalSizeVertex,
        meshArray,
        scene->mRootNode,
        scene
    );

    vertexArray.reserve(totalSizeVertex);
    indexArray.reserve(totalSizeIndex);

    // We add all shapes of the model into the same vertex array, one after the other.
    // Here we are also including more than a mesh when they are present. This should
    // be avoided if possible.
    unsigned int positionIndex = 0;
    //size_t indicesIndex = 0;

    for (const aiMesh* meshPtr : meshArray) {
        // Walk through each of the mesh's vertices
        for (unsigned int i = 0; i < meshPtr->mNumVertices; i++) {
            Vertex vTmp {};

            // Positions
            vTmp.location[0] = meshPtr->mVertices[i].x;
            vTmp.location[1] = meshPtr->mVertices[i].y;
            vTmp.location[2] = meshPtr->mVertices[i].z;

            // Normals
            vTmp.normal[0] = meshPtr->mNormals[i].x;
            vTmp.normal[1] = meshPtr->mNormals[i].y;
            vTmp.normal[2] = meshPtr->mNormals[i].z;

            // Texture Coordinates
            if (meshPtr->mTextureCoords[0]) {
                // Each vertex can have at most 8 different texture coordinates.
                // We are using only the first one provided.
                vTmp.tex[0] = meshPtr->mTextureCoords[0][i].x;
                vTmp.tex[1] = meshPtr->mTextureCoords[0][i].y;
            }
            else {
                vTmp.tex[0] = 0.0;
                vTmp.tex[1] = 0.0;
            }

            vertexArray.push_back(vTmp);
        }

        // Walking through the mesh faces and get the vertexes indices
        for (unsigned int nf = 0; nf < meshPtr->mNumFaces; ++nf) {
            const struct aiFace* face = &meshPtr->mFaces[nf];

            if (face->mNumIndices == 3) {
                for (unsigned int ii = 0; ii < face->mNumIndices; ii++) {
                    indexArray.push_back(
                        static_cast<GLint>(face->mIndices[ii] + positionIndex)
                    );
                }
            }
        }

        // Process materials (Not in use now)
        //if (meshPtr->mMaterialIndex >= 0)
        //{
        //    aiMaterial* material = scene->mMaterials[meshPtr->mMaterialIndex];
        //    // We assume a convention for sampler names in the shaders. Each diffuse
        //    // texture should be named as 'texture_diffuseN' where N is a sequential
        //    // number ranging from 1 to MAX_SAMPLER_NUMBER.
        //    // Same applies to other texture as the following list summarizes:
        //    // Diffuse: texture_diffuseN
        //    // Specular: texture_specularN
        //    // Normal: texture_normalN

        //    // 1. Diffuse maps
        //    vector<Texture> diffuseMaps = this->loadMaterialTextures(
        //        material,
        //        aiTextureType_DIFFUSE,
        //        "texture_diffuse"
        //    );
        //    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        //    // 2. Specular maps
        //    vector<Texture> specularMaps = this->loadMaterialTextures(
        //        material,
        //        aiTextureType_SPECULAR,
        //        "texture_specular"
        //    );
        //    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        //    // 3. Reflection maps (Note that ASSIMP doesn't load reflection maps
        //    .. properly from wavefront objects, so we'll cheat a little by defining the
        //    // reflection maps as ambient maps in the .obj file, which ASSIMP is able to
        //    // load)
        //    vector<Texture> reflectionMaps = this->loadMaterialTextures(
        //        material,
        //        aiTextureType_AMBIENT,
        //        "texture_reflection"
        //    );
        //    textures.insert(
        //        textures.end(),
        //        reflectionMaps.begin(),
        //        reflectionMaps.end()
        //    );
        //}

        positionIndex += meshPtr->mNumVertices;
    }
}

} // namespace ghoul::io
