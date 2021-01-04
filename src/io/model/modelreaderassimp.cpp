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

#define FORMAT_STRING_SIZE 4

#include <ghoul/io/model/modelreaderassimp.h>

#include <ghoul/io/model/modelgeometry.h>
#include <ghoul/io/model/modelmesh.h>
#include <ghoul/io/texture/texturereader.h>
#include <ghoul/io/texture/texturereaderbase.h>
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
#include <fstream>

namespace {
    constexpr const char* _loggerCat = "ModelReaderAssimp";
    constexpr int8_t CurrentCacheVersion = 1;
}

namespace ghoul::io {

void loadMaterialTextures(const aiScene* scene, aiMaterial* mat, aiTextureType type,
                          std::string textureType,
                          std::vector<ModelMesh::Texture>& textureArray,
                 std::vector<modelgeometry::ModelGeometry::TextureEntry>& textureStorage)
{
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        ModelMesh::Texture textureTmp;
        textureTmp.type = textureType;
        aiString pathString;
        mat->GetTexture(type, i, &pathString);

        // Check for duplicates
        bool shouldSkip = false;
        for (unsigned int j = 0; j < textureArray.size(); ++j) {
            if (textureArray[j].hasTexture) {
                int isDuplicate = std::strcmp(
                    textureArray[j].texture->name().c_str(),
                    pathString.C_Str()
                );
                if (isDuplicate == 0) {
                    // Texture has already been loaded for this mesh, continue to next one
                    shouldSkip = true;
                    break;
                }
            }
        }

        // Texture already loaded, do not load it again
        if (shouldSkip)
            continue;

        // Check if texture has already been loaded by other meshes
        for (unsigned int j = 0; j < textureStorage.size(); ++j) {
            int isLoaded = std::strcmp(textureStorage[j].name.c_str(), pathString.C_Str());
            if (isLoaded == 0) {
                // Texture has already been loaded. Point to that texture instead of copying
                textureTmp.texture = textureStorage[j].texture.get();
                textureTmp.texture->setName(pathString.C_Str());
                textureTmp.hasTexture = true;
                textureArray.push_back(std::move(textureTmp));
                shouldSkip = true;
                break;
            }
        }

        // Texture already loaded, do not load it again
        if (shouldSkip)
            continue;

        // Load texture
        // Check if the texture is an embedded texture and whether it is compressed or not
        const aiTexture* texture = scene->GetEmbeddedTexture(pathString.C_Str());
        modelgeometry::ModelGeometry::TextureEntry textureEntry;
        textureEntry.name = pathString.C_Str();
        if (texture) {
            // Embedded texture
            if ((texture->mHeight == 0)) {
                // Load compressed embedded texture
                try {
                    textureEntry.texture = TextureReader::ref().loadTexture(
                        static_cast<void*>(texture->pcData),
                        texture->mWidth,
                        texture->achFormatHint
                    );
                    textureTmp.texture = textureEntry.texture.get();
                    textureTmp.texture->setName(pathString.C_Str());
                    LDEBUG(
                        fmt::format(
                            "Loaded texture from '{}'",
                            static_cast<void*>(texture->pcData)
                        )
                    );
                }
                catch (TextureReader::MissingReaderException e) {
                    LWARNING(
                        fmt::format(
                            "Failed to load texture from '{}' with extension '{}' : "
                            "Replacing with flashy color.",
                            e.file,
                            e.fileExtension
                        )
                    );
                    textureTmp.texture = nullptr;
                    textureTmp.hasTexture = false;
                    textureTmp.useForcedColor = true;
                    textureArray.push_back(std::move(textureTmp));
                    continue;
                }
                catch (TextureReaderBase::TextureLoadException e) {
                    LWARNING(
                        fmt::format(
                            "Failed to load texture from '{}' with error: '{}' : "
                            "Replacing with flashy color.",
                            e.filename,
                            e.message
                        )
                    );
                    textureTmp.texture = nullptr;
                    textureTmp.hasTexture = false;
                    textureTmp.useForcedColor = true;
                    textureArray.push_back(std::move(textureTmp));
                    continue;
                }
            }
            else {
                // Load uncompressed embedded texture
                LWARNING("Uncompressed embedded texture detected: Not supported! "
                    "Replacing with flashy color."
                );
                textureTmp.texture = nullptr;
                textureTmp.hasTexture = false;
                textureTmp.useForcedColor = true;
                textureArray.push_back(std::move(textureTmp));
                continue;
            }
        }
        else {
            // Local texture
            try {
                textureEntry.texture = TextureReader::ref().loadTexture(
                    absPath(pathString.C_Str())
                );
                textureTmp.texture = textureEntry.texture.get();
                textureTmp.texture->setName(pathString.C_Str());
                LDEBUG(
                    fmt::format("Loaded texture from '{}'", absPath(pathString.C_Str()))
                );
            }
            catch (TextureReader::MissingReaderException e) {
                LWARNING(
                    fmt::format(
                        "Failed to load texture from '{}' with extension '{}' : "
                        "Replacing with flashy color.",
                        e.file,
                        e.fileExtension
                    )
                );
                textureTmp.texture = nullptr;
                textureTmp.hasTexture = false;
                textureTmp.useForcedColor = true;
                textureArray.push_back(std::move(textureTmp));
                continue;
            }
            catch (TextureReaderBase::TextureLoadException e) {
                LWARNING(
                    fmt::format(
                        "Failed to load texture from '{}' with error: '{}' : "
                        "Replacing with flashy color.",
                        e.filename,
                        e.message
                    )
                );
                textureTmp.texture = nullptr;
                textureTmp.hasTexture = false;
                textureTmp.useForcedColor = true;
                textureArray.push_back(std::move(textureTmp));
                continue;
            }
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
            continue;

        // Add new Texture to the textureStorage and point to it in the texture array
        textureTmp.hasTexture = true;
        textureArray.push_back(std::move(textureTmp));
        textureStorage.push_back(std::move(textureEntry));
    }
}


ModelMesh processMesh(aiMesh* mesh, const aiScene* scene, glm::mat4x4& transform,
                 std::vector<modelgeometry::ModelGeometry::TextureEntry>& textureStorage)
{
    std::vector<ModelMesh::Vertex> vertexArray;
    std::vector<unsigned int> indexArray;
    std::vector<ModelMesh::Texture> textureArray;

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
            textureArray,
            textureStorage
        );
    }
    else {
        // Load embedded simple material instead of textures
        aiReturn hasColor = material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        if (hasColor == AI_SUCCESS) {
            ModelMesh::Texture textureTmp;
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
            textureArray,
            textureStorage
        );
    }
    else {
        // Load embedded simple material instead of textures
        color = aiColor3D(0.f, 0.f, 0.f);
        aiReturn hasColor = material->Get(AI_MATKEY_COLOR_SPECULAR, color);
        if (hasColor == AI_SUCCESS && !color.IsBlack()) {
            ModelMesh::Texture textureTmp;
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
            textureArray,
            textureStorage
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
                 const glm::mat4x4& parentTransform,
                 std::vector<modelgeometry::ModelGeometry::TextureEntry>& textureStorage,
                 const bool forceRenderInvisible, const bool notifyInvisibleDropped)
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
        ModelMesh loadedMesh = processMesh(mesh, scene, globalTransform, textureStorage);

        // If mesh is invisible (no materials) drop it unless forced to render anyway
        // Notify unless suppresed
        if (loadedMesh._textures.empty()) {
            // Force invisible mesh to render with flashy colors
            if (forceRenderInvisible) {
                ModelMesh::Texture textureTmp;
                textureTmp.hasTexture = false;
                textureTmp.useForcedColor = true;
                loadedMesh._textures.push_back(std::move(textureTmp));
            }
            // If not forced to render, drop invisible mesh
            else {
                if (notifyInvisibleDropped) {
                    LINFO(fmt::format("Invisible mesh '{}' dropped", mesh->mName.C_Str()));
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
            textureStorage,
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
    const aiScene* scene = importer.ReadFile(
        filename,
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
    std::vector<modelgeometry::ModelGeometry::TextureEntry> textureStorage;
    processNode(
        scene->mRootNode,
        scene,
        meshArray,
        rootTransform,
        textureStorage,
        forceRenderInvisible,
        notifyInvisibleDropped
    );

    // Return the ModelGeometry from the meshArray
    return std::make_unique<modelgeometry::ModelGeometry>(
        std::move(modelgeometry::ModelGeometry(
            std::move(meshArray),
            std::move(textureStorage)
        ))
    );
}

const opengl::Texture::Format formatFromString(const std::string format) {
    if (format == "Red ") {
        return opengl::Texture::Format::Red;
    }
    else if (format == "RG  ") {
        return opengl::Texture::Format::RG;
    }
    else if (format == "RGB ") {
        return opengl::Texture::Format::RGB;
    }
    else if (format == "BGR ") {
        return opengl::Texture::Format::BGR;
    }
    else if (format == "RGBA") {
        return opengl::Texture::Format::RGBA;
    }
    else if (format == "BGRA") {
        return opengl::Texture::Format::BGRA;
    }
    else if (format == "Dept") {
        return opengl::Texture::Format::DepthComponent;
    }
    else {
        throw MissingCaseException();
    }
}

std::string stringFromFormat(opengl::Texture::Format format) {
    std::string formatString;
    formatString.resize(FORMAT_STRING_SIZE, ' ');
    std::string subString;

    switch (format) {
    case opengl::Texture::Format::Red:
        subString = "Red";
        break;
    case opengl::Texture::Format::RG:
        subString = "RG";
        break;
    case opengl::Texture::Format::RGB:
        subString = "RGB";
        break;
    case opengl::Texture::Format::BGR:
        subString = "BGR";
        break;
    case opengl::Texture::Format::RGBA:
        subString = "RGBA";
        break;
    case opengl::Texture::Format::BGRA:
        subString = "BGRA";
        break;
    case opengl::Texture::Format::DepthComponent:
        subString = "Dept";
        break;
    default:
        throw MissingCaseException();
    }

    formatString.replace(0, subString.length(), subString);
    return formatString;
}

const GLenum dataTypeFromString(const std::string dataType) {
    if (dataType == "byte") {
        return GL_BYTE;
    }
    else if (dataType == "ubyt") {
        return GL_UNSIGNED_BYTE;
    }
    else if (dataType == "shor") {
        return GL_SHORT;
    }
    else if (dataType == "usho") {
        return GL_UNSIGNED_SHORT;
    }
    else if (dataType == "int ") {
        return GL_INT;
    }
    else if (dataType == "uint") {
        return GL_UNSIGNED_INT;
    }
    else if (dataType == "floa") {
        return GL_FLOAT;
    }
    else if (dataType == "doub") {
        return GL_DOUBLE;
    }
    else {
        throw MissingCaseException();
    }
}

std::string stringFromDataType(const GLenum dataType) {
    std::string formatString;
    formatString.resize(FORMAT_STRING_SIZE, ' ');
    std::string subString;

    switch (dataType) {
        case GL_BYTE :
            subString = "byte";
            break;
        case GL_UNSIGNED_BYTE :
            subString = "ubyt";
            break;
        case GL_SHORT :
            subString = "shor";
            break;
        case GL_UNSIGNED_SHORT :
            subString = "usho";
            break;
        case GL_INT :
            subString = "int";
            break;
        case GL_UNSIGNED_INT :
            subString = "uint";
            break;
        case GL_FLOAT :
            subString = "floa";
            break;
        case GL_DOUBLE :
            subString = "doub";
            break;
        default :
            throw MissingCaseException();
    }

    formatString.replace(0, subString.length(), subString);
    return formatString;
}

std::unique_ptr<modelgeometry::ModelGeometry> ModelReaderAssimp::loadCachedFile(
                                                     const std::string& cachedFile) const
{
    std::ifstream fileStream(cachedFile, std::ifstream::binary);
    if (!fileStream.good()) {
        throw ModelLoadException(cachedFile, "Could not open file", this);
    }

    // Check the caching version
    int8_t version = 0;
    fileStream.read(reinterpret_cast<char*>(&version), sizeof(int8_t));
    if (version != CurrentCacheVersion) {
        fileStream.close();
        throw ModelLoadException(
            cachedFile,
            "The format of the cached file has changed: deleting old cache",
            this
        );
    }

    // First read the textureEntries
    int32_t nTextureEntries = 0;
    fileStream.read(reinterpret_cast<char*>(&nTextureEntries), sizeof(int32_t));
    if (nTextureEntries == 0) {
        LINFO("No TextureEntries were loaded!");
    }
    std::vector<modelgeometry::ModelGeometry::TextureEntry> textureStorageArray;
    textureStorageArray.reserve(nTextureEntries);

    for (unsigned int te = 0; te < nTextureEntries; ++te) {
        modelgeometry::ModelGeometry::TextureEntry textureEntry;

        // Name
        int32_t nameSize = 0;
        fileStream.read(reinterpret_cast<char*>(&nameSize), sizeof(int32_t));
        if (nameSize == 0) {
            throw ModelLoadException(cachedFile, "No texture name was loaded", this);
        }
        textureEntry.name.resize(nameSize);
        fileStream.read(textureEntry.name.data(), nameSize);

        // Texture
        // dimensions
        glm::uvec3 dimensions;
        fileStream.read(reinterpret_cast<char*>(&dimensions.x), sizeof(int32_t));
        fileStream.read(reinterpret_cast<char*>(&dimensions.y), sizeof(int32_t));
        fileStream.read(reinterpret_cast<char*>(&dimensions.z), sizeof(int32_t));

        // format
        std::string formatString;
        formatString.resize(FORMAT_STRING_SIZE);
        fileStream.read(formatString.data(), FORMAT_STRING_SIZE * sizeof(char));
        opengl::Texture::Format format = formatFromString(formatString);

        // internal format
        unsigned int rawInternalFormat;
        fileStream.read(reinterpret_cast<char*>(
            &rawInternalFormat),
            sizeof(unsigned int)
        );
        GLenum internalFormat = static_cast<GLenum>(rawInternalFormat);

        // data type
        std::string dataTypeString;
        dataTypeString.resize(FORMAT_STRING_SIZE);
        fileStream.read(dataTypeString.data(), FORMAT_STRING_SIZE * sizeof(char));
        GLenum dataType = dataTypeFromString(dataTypeString);

        // data
        int32_t textureSize = 0;
        fileStream.read(reinterpret_cast<char*>(&textureSize), sizeof(int32_t));
        if (textureSize == 0) {
            throw ModelLoadException(
                cachedFile,
                "No texture size was loaded",
                this
            );
        }
        std::byte* data = new std::byte[textureSize];
        fileStream.read(reinterpret_cast<char*>(data), textureSize);

        textureEntry.texture =
            std::make_unique<opengl::Texture>(
                dimensions,
                format,
                internalFormat,
                dataType,
                opengl::Texture::FilterMode::Linear,
                opengl::Texture::WrappingMode::Repeat,
                opengl::Texture::AllocateData::No,
                opengl::Texture::TakeOwnership::Yes
            );

        textureEntry.texture->setPixelData(data);

        textureStorageArray.push_back(std::move(textureEntry));
    }

    // Read how many meshes to read
    int32_t nMeshes = 0;
    fileStream.read(reinterpret_cast<char*>(&nMeshes), sizeof(int32_t));
    if (nMeshes == 0) {
        throw ModelLoadException(cachedFile, "No meshes were loaded", this);
    }

    std::vector<ModelMesh> meshArray;
    meshArray.reserve(nMeshes);

    // Read the meshes in same order as they were written
    for (unsigned int m = 0; m < nMeshes; ++m) {
        // Vertices
        int32_t nVertices = 0;
        fileStream.read(reinterpret_cast<char*>(&nVertices), sizeof(int32_t));
        if (nVertices == 0) {
            throw ModelLoadException(cachedFile, "No vertices were loaded", this);
        }
        std::vector<ModelMesh::Vertex> vertexArray;
        vertexArray.reserve(nVertices);

        for (unsigned int v = 0; v < nVertices; ++v) {
            ModelMesh::Vertex vertex;
            fileStream.read(reinterpret_cast<char*>(&vertex), sizeof(ModelMesh::Vertex));
            vertexArray.push_back(std::move(vertex));
        }

        // Indices
        int32_t nIndices = 0;
        fileStream.read(reinterpret_cast<char*>(&nIndices), sizeof(int32_t));
        if (nIndices == 0) {
            throw ModelLoadException(cachedFile, "No indices were loaded", this);
        }
        std::vector<unsigned int> indexArray;
        indexArray.reserve(nIndices);

        for (unsigned int i = 0; i < nIndices; ++i) {
            unsigned int index;
            fileStream.read(reinterpret_cast<char*>(&index), sizeof(unsigned int));
            indexArray.push_back(index);
        }

        // Textures
        int32_t nTextures = 0;
        fileStream.read(reinterpret_cast<char*>(&nTextures), sizeof(int32_t));
        if (nTextures == 0) {
            throw ModelLoadException(cachedFile, "No textures were loaded", this);
        }
        std::vector<ModelMesh::Texture> textureArray;
        textureArray.reserve(nTextures);

        for (unsigned int t = 0; t < nTextures; ++t) {
            ModelMesh::Texture texture;

            // type
            int32_t typeSize = 0;
            fileStream.read(reinterpret_cast<char*>(&typeSize), sizeof(int32_t));
            if (typeSize == 0) {
                throw ModelLoadException(cachedFile, "No texture type was loaded", this);
            }
            texture.type.resize(typeSize);
            fileStream.read(texture.type.data(), typeSize);

            // hasTexture
            fileStream.read(reinterpret_cast<char*>(&texture.hasTexture), sizeof(bool));

            // useForcedColor
            fileStream.read(reinterpret_cast<char*>(
                &texture.useForcedColor),
                sizeof(bool)
            );

            // color
            fileStream.read(reinterpret_cast<char*>(&texture.color.r), sizeof(float));
            fileStream.read(reinterpret_cast<char*>(&texture.color.g), sizeof(float));
            fileStream.read(reinterpret_cast<char*>(&texture.color.b), sizeof(float));

            // texture
            if (texture.hasTexture) {
                // Read which index in the textureStorageArray that this texture should point to
                unsigned int index;
                fileStream.read(reinterpret_cast<char*>(&index), sizeof(unsigned int));

                if (index >= textureStorageArray.size()) {
                    throw ModelLoadException(
                        cachedFile,
                        "Texture index is outside of textureStorage",
                        this
                    );
                }

                texture.texture = textureStorageArray[index].texture.get();
            }
            textureArray.push_back(std::move(texture));
        }

        // Make mesh
        meshArray.push_back(std::move(ModelMesh(
                std::move(vertexArray),
                std::move(indexArray),
                std::move(textureArray)
            ))
        );
    }

    // Create the ModelGeometry
    return std::make_unique<modelgeometry::ModelGeometry>(
        std::move(modelgeometry::ModelGeometry(
            std::move(meshArray),
            std::move(textureStorageArray)
        ))
    );
}

bool ModelReaderAssimp::saveCachedFile(const std::string& cachedFile,
                                       const modelgeometry::ModelGeometry* model) const
{
    std::ofstream fileStream(cachedFile, std::ofstream::binary);
    if (!fileStream.good()) {
        throw ModelSaveException(
            cachedFile,
            fmt::format("Error opening file '{}' for saving model cache", cachedFile),
            this
        );
    }

    // Write which version of caching that is used
    fileStream.write(
        reinterpret_cast<const char*>(&CurrentCacheVersion),
        sizeof(int8_t)
    );

    // First cache the textureStorage
    int32_t nTextureEntries = model->textureStorage().size();
    if (nTextureEntries == 0) {
        LINFO("No TextureEntries were loaded!");
    }
    fileStream.write(reinterpret_cast<const char*>(&nTextureEntries), sizeof(int32_t));

    for (unsigned int te = 0; te < nTextureEntries; ++te) {
        // Name
        int32_t nameSize = model->textureStorage()[te].name.size() * sizeof(char);
        if (nameSize == 0) {
            throw ModelSaveException(
                cachedFile,
                "Error writing cache: No texture name was loaded",
                this
            );
        }
        fileStream.write(reinterpret_cast<const char*>(&nameSize), sizeof(int32_t));
        fileStream.write(model->textureStorage()[te].name.data(), nameSize);

        // Texture
        // dimensions
        fileStream.write(reinterpret_cast<const char*>(
            &model->textureStorage()[te].texture->dimensions().x),
            sizeof(int32_t)
        );
        fileStream.write(reinterpret_cast<const char*>(
            &model->textureStorage()[te].texture->dimensions().y),
            sizeof(int32_t)
        );
        fileStream.write(reinterpret_cast<const char*>(
            &model->textureStorage()[te].texture->dimensions().z),
            sizeof(int32_t)
        );

        // format
        std::string format =
            stringFromFormat(model->textureStorage()[te].texture->format());
        fileStream.write(format.data(), FORMAT_STRING_SIZE * sizeof(char));

        // internal format
        unsigned int internalFormat = static_cast<unsigned int>(
            model->textureStorage()[te].texture->internalFormat()
        );
        fileStream.write(
            reinterpret_cast<const char*>(&internalFormat),
            sizeof(unsigned int)
        );

        // data type
        std::string dataType =
            stringFromDataType(model->textureStorage()[te].texture->dataType());
        fileStream.write(dataType.data(), FORMAT_STRING_SIZE * sizeof(char));

        // data
        model->textureStorage()[te].texture->downloadTexture();
        int32_t pixelSize = model->textureStorage()[te].texture->expectedPixelDataSize();
        if (pixelSize == 0) {
            throw ModelSaveException(
                cachedFile,
                "Error writing cache: No texture size loaded",
                this
            );
        }
        fileStream.write(reinterpret_cast<const char*>(&pixelSize), sizeof(int32_t));

        const void* data = model->textureStorage()[te].texture->pixelData();
        fileStream.write(reinterpret_cast<const char*>(data), pixelSize);
    }

    // Write how many meshes are to be written
    int32_t nMeshes = model->meshes().size();
    if (nMeshes == 0) {
        throw ModelSaveException(
            cachedFile,
            "Error writing cache: No meshes were loaded",
            this
        );
    }
    fileStream.write(reinterpret_cast<const char*>(&nMeshes), sizeof(int32_t));

    // Meshes
    for (unsigned int m = 0; m < nMeshes; m++) {
        // Vertices
        int32_t nVertices = model->meshes()[m]._vertices.size();
        if (nVertices == 0) {
            throw ModelSaveException(
                cachedFile,
                "Error writing cache: No vertices were loaded",
                this
            );
        }
        fileStream.write(reinterpret_cast<const char*>(&nVertices), sizeof(int32_t));

        for (unsigned int v = 0; v < nVertices; ++v) {
            fileStream.write(
                reinterpret_cast<const char*>(&model->meshes()[m]._vertices[v]),
                sizeof(ModelMesh::Vertex)
            );
        }

        // Indices
        int32_t nIndices = model->meshes()[m]._indices.size();
        if (nIndices == 0) {
            throw ModelSaveException(
                cachedFile,
                "Error writing cache: No indices were loaded",
                this
            );
        }
        fileStream.write(reinterpret_cast<const char*>(&nIndices), sizeof(int32_t));

        for (unsigned int i = 0; i < nIndices; ++i) {
            fileStream.write(reinterpret_cast<const char*>(
                &model->meshes()[m]._indices[i]),
                sizeof(unsigned int)
            );
        }

        // Textures
        int32_t nTextures = model->meshes()[m]._textures.size();
        if (nTextures == 0) {
            throw ModelSaveException(
                cachedFile,
                "Error writing cache: No textures were loaded",
                this
            );
        }
        fileStream.write(reinterpret_cast<const char*>(&nTextures), sizeof(int32_t));

        for (unsigned int t = 0; t < nTextures; ++t) {
            // type
            int32_t typeSize =
                model->meshes()[m]._textures[t].type.size() * sizeof(char);
            if (typeSize == 0) {
                throw ModelSaveException(
                    cachedFile,
                    "Error writing cache: No texture type loaded",
                    this
                );
            }
            fileStream.write(reinterpret_cast<const char*>(&typeSize), sizeof(int32_t));
            fileStream.write(model->meshes()[m]._textures[t].type.data(), typeSize);

            // hasTexture
            fileStream.write(reinterpret_cast<const char*>(
                &model->meshes()[m]._textures[t].hasTexture),
                sizeof(bool)
            );

            // useForcedColor
            fileStream.write(reinterpret_cast<const char*>(
                &model->meshes()[m]._textures[t].useForcedColor),
                sizeof(bool)
            );

            // color
            fileStream.write(reinterpret_cast<const char*>(
                &model->meshes()[m]._textures[t].color.r),
                sizeof(float)
            );
            fileStream.write(reinterpret_cast<const char*>(
                &model->meshes()[m]._textures[t].color.g),
                sizeof(float)
            );
            fileStream.write(reinterpret_cast<const char*>(
                &model->meshes()[m]._textures[t].color.b),
                sizeof(float)
            );

            // texture
            if (model->meshes()[m]._textures[t].hasTexture) {
                // Search the textureStorage to find which entry this texture points to
                int wasFound = -1;
                for (unsigned int te = 0; te < model->textureStorage().size(); ++te) {
                    wasFound = std::strcmp(
                        model->textureStorage()[te].name.c_str(),
                        model->meshes()[m]._textures[t].texture->name().c_str()
                    );
                    if (wasFound == 0) {
                        fileStream.write(
                            reinterpret_cast<const char*>(&te),
                            sizeof(unsigned int)
                        );
                        break;
                    }
                }

                if (wasFound != 0) {
                    throw ModelSaveException(
                        cachedFile,
                        "Error writing cache: Could not find texture in textureStorage",
                        this
                    );
                }
            }
        }
    }

    return fileStream.good();
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
