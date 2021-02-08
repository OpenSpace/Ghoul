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
    constexpr const int8_t CurrentCacheVersion = 1;
    constexpr const int FormatStringSize = 4;
} // namespace

namespace ghoul::io {

void generateDebugTexture(ModelMesh::Texture& texture) {
    texture.texture = nullptr;
    texture.hasTexture = false;
    texture.useForcedColor = true;
    texture.type = "color_diffuse";
}

bool loadMaterialTextures(const aiScene& scene, const aiMaterial& material,
                          const aiTextureType& type, const std::string& typeString,
                          std::vector<ModelMesh::Texture>& textureArray,
                 std::vector<modelgeometry::ModelGeometry::TextureEntry>& textureStorage,
                          std::filesystem::path& modelDirectory)
{
    for (unsigned int i = 0; i < material.GetTextureCount(type); ++i) {
        ModelMesh::Texture meshTexture;
        meshTexture.type = typeString;
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
                      std::filesystem::path& modelDirectory)
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
            "texture_diffuse",
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
                texture.type = "color_diffuse";
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
                texture.type = "color_diffuse";
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
            "texture_specular",
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
                texture.type = "color_specular";
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
                texture.type = "color_specular";
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
            "texture_normal",
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
            modelDirectory
        );

        // If mesh is invisible (no materials) drop it unless forced to render anyway
        // Notify unless suppresed
        if (loadedMesh._textures.empty()) {
            if (forceRenderInvisible) {
                // Force invisible mesh to render with flashy colors
                ModelMesh::Texture texture;
                generateDebugTexture(texture);
                loadedMesh._textures.push_back(std::move(texture));
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

opengl::Texture::Format stringToFormat(std::string_view format) {
    if (format == "Red ") { return opengl::Texture::Format::Red; }
    else if (format == "RG  ") { return opengl::Texture::Format::RG; }
    else if (format == "RGB ") { return opengl::Texture::Format::RGB; }
    else if (format == "BGR ") { return opengl::Texture::Format::BGR; }
    else if (format == "RGBA") { return opengl::Texture::Format::RGBA; }
    else if (format == "BGRA") { return opengl::Texture::Format::BGRA; }
    else if (format == "Dept") { return opengl::Texture::Format::DepthComponent; }
    else { throw MissingCaseException(); }
}

std::string formatToString(opengl::Texture::Format format) {
    switch (format) {
        case opengl::Texture::Format::Red: return "Red ";
        case opengl::Texture::Format::RG: return "RG  ";
        case opengl::Texture::Format::RGB: return "RGB ";
        case opengl::Texture::Format::BGR: return "BGR ";
        case opengl::Texture::Format::RGBA: return "RGBA";
        case opengl::Texture::Format::BGRA: return "BGRA";
        case opengl::Texture::Format::DepthComponent: return "Dept";
        default: throw MissingCaseException();
    }
}

GLenum stringToDataType(std::string_view dataType) {
    if (dataType == "byte") { return GL_BYTE; }
    else if (dataType == "ubyt") { return GL_UNSIGNED_BYTE; }
    else if (dataType == "shor") { return GL_SHORT; }
    else if (dataType == "usho") { return GL_UNSIGNED_SHORT; }
    else if (dataType == "int ") { return GL_INT; }
    else if (dataType == "uint") { return GL_UNSIGNED_INT; }
    else if (dataType == "floa") { return GL_FLOAT; }
    else if (dataType == "doub") { return GL_DOUBLE; }
    else { throw MissingCaseException(); }
}

std::string dataTypeToString(GLenum dataType) {
    switch (dataType) {
        case GL_BYTE: return "byte";
        case GL_UNSIGNED_BYTE: return "ubyt";
        case GL_SHORT: return "shor";
        case GL_UNSIGNED_SHORT: return "usho";
        case GL_INT: return "int ";
        case GL_UNSIGNED_INT: return "uint";
        case GL_FLOAT: return "floa";
        case GL_DOUBLE: return "doub";
        default: throw MissingCaseException();
    }
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
        LINFO("No TextureEntries were loaded");
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
        std::array<int32_t, 3> dimensionStorage;
        fileStream.read(reinterpret_cast<char*>(dimensionStorage.data()), 3 * sizeof(int32_t));
        glm::uvec3 dimensions = glm::uvec3(
            static_cast<unsigned int>(dimensionStorage[0]),
            static_cast<unsigned int>(dimensionStorage[1]),
            static_cast<unsigned int>(dimensionStorage[2])
        );

        // format
        std::string formatString;
        formatString.resize(FormatStringSize);
        fileStream.read(formatString.data(), FormatStringSize * sizeof(char));
        opengl::Texture::Format format = stringToFormat(formatString);

        // internal format
        uint32_t rawInternalFormat;
        fileStream.read(reinterpret_cast<char*>(&rawInternalFormat), sizeof(uint32_t));
        GLenum internalFormat = static_cast<GLenum>(rawInternalFormat);

        // data type
        std::string dataTypeString;
        dataTypeString.resize(FormatStringSize);
        fileStream.read(dataTypeString.data(), FormatStringSize * sizeof(char));
        GLenum dataType = stringToDataType(dataTypeString);

        // data
        int32_t textureSize = 0;
        fileStream.read(reinterpret_cast<char*>(&textureSize), sizeof(int32_t));
        if (textureSize == 0) {
            throw ModelLoadException(cachedFile, "No texture size was loaded", this);
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

        textureEntry.texture->setPixelData(data, opengl::Texture::TakeOwnership::Yes);
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
            uint32_t index;
            fileStream.read(reinterpret_cast<char*>(&index), sizeof(uint32_t));
            indexArray.push_back(std::move(static_cast<unsigned int>(index)));
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
            uint8_t h;
            fileStream.read(reinterpret_cast<char*>(&h), sizeof(uint8_t));
            texture.hasTexture = static_cast<bool>(h);

            // useForcedColor
            uint8_t f;
            fileStream.read(
                reinterpret_cast<char*>(&f),
                sizeof(uint8_t)
            );
            texture.useForcedColor = static_cast<bool>(f);

            // color
            fileStream.read(reinterpret_cast<char*>(&texture.color.r), sizeof(float));
            fileStream.read(reinterpret_cast<char*>(&texture.color.g), sizeof(float));
            fileStream.read(reinterpret_cast<char*>(&texture.color.b), sizeof(float));

            // texture
            if (texture.hasTexture) {
                // Read which index in the textureStorageArray that this texture should point to
                uint32_t index;
                fileStream.read(reinterpret_cast<char*>(&index), sizeof(uint32_t));

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
        meshArray.push_back(ModelMesh(
            std::move(vertexArray),
            std::move(indexArray),
            std::move(textureArray)
        ));
    }

    // Create the ModelGeometry
    return std::make_unique<modelgeometry::ModelGeometry>(
        std::move(meshArray),
        std::move(textureStorageArray)
    );
}

bool ModelReaderAssimp::saveCachedFile(const std::string& cachedFile,
                                       const modelgeometry::ModelGeometry& model) const
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
    int32_t nTextureEntries = model.textureStorage().size();
    if (nTextureEntries == 0) {
        LINFO("No TextureEntries were loaded");
    }
    fileStream.write(reinterpret_cast<const char*>(&nTextureEntries), sizeof(int32_t));

    for (unsigned int te = 0; te < nTextureEntries; ++te) {
        // Name
        int32_t nameSize = model.textureStorage()[te].name.size() * sizeof(char);
        if (nameSize == 0) {
            throw ModelSaveException(
                cachedFile,
                "Error writing cache: No texture name was loaded",
                this
            );
        }
        fileStream.write(reinterpret_cast<const char*>(&nameSize), sizeof(int32_t));
        fileStream.write(model.textureStorage()[te].name.data(), nameSize);

        // Texture
        // dimensions
        std::array<int32_t, 3> dimensionStorage;
        dimensionStorage[0] = model.textureStorage()[te].texture->dimensions().x;
        dimensionStorage[1] = model.textureStorage()[te].texture->dimensions().y;
        dimensionStorage[2] = model.textureStorage()[te].texture->dimensions().z;

        fileStream.write(
            reinterpret_cast<const char*>(dimensionStorage.data()),
            3 * sizeof(int32_t)
        );

        // format
        std::string format =
            formatToString(model.textureStorage()[te].texture->format());
        fileStream.write(format.data(), FormatStringSize * sizeof(char));

        // internal format
        uint32_t internalFormat = static_cast<uint32_t>(
            model.textureStorage()[te].texture->internalFormat()
        );
        fileStream.write(
            reinterpret_cast<const char*>(&internalFormat),
            sizeof(uint32_t)
        );

        // data type
        std::string dataType =
            dataTypeToString(model.textureStorage()[te].texture->dataType());
        fileStream.write(dataType.data(), FormatStringSize * sizeof(char));

        // data
        model.textureStorage()[te].texture->downloadTexture();
        int32_t pixelSize = model.textureStorage()[te].texture->expectedPixelDataSize();
        if (pixelSize == 0) {
            throw ModelSaveException(
                cachedFile,
                "Error writing cache: No texture size loaded",
                this
            );
        }
        fileStream.write(reinterpret_cast<const char*>(&pixelSize), sizeof(int32_t));

        const void* data = model.textureStorage()[te].texture->pixelData();
        fileStream.write(reinterpret_cast<const char*>(data), pixelSize);
    }

    // Write how many meshes are to be written
    int32_t nMeshes = model.meshes().size();
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
        int32_t nVertices = model.meshes()[m]._vertices.size();
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
                reinterpret_cast<const char*>(&model.meshes()[m]._vertices[v]),
                sizeof(ModelMesh::Vertex)
            );
        }

        // Indices
        int32_t nIndices = model.meshes()[m]._indices.size();
        if (nIndices == 0) {
            throw ModelSaveException(
                cachedFile,
                "Error writing cache: No indices were loaded",
                this
            );
        }
        fileStream.write(reinterpret_cast<const char*>(&nIndices), sizeof(int32_t));

        for (unsigned int i = 0; i < nIndices; ++i) {
            uint32_t index = static_cast<uint32_t>(model.meshes()[m]._indices[i]);
            fileStream.write(reinterpret_cast<const char*>(&index), sizeof(uint32_t));
        }

        // Textures
        int32_t nTextures = model.meshes()[m]._textures.size();
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
                model.meshes()[m]._textures[t].type.size() * sizeof(char);
            if (typeSize == 0) {
                throw ModelSaveException(
                    cachedFile,
                    "Error writing cache: No texture type loaded",
                    this
                );
            }
            fileStream.write(reinterpret_cast<const char*>(&typeSize), sizeof(int32_t));
            fileStream.write(model.meshes()[m]._textures[t].type.data(), typeSize);

            // hasTexture
            uint8_t h = (model.meshes()[m]._textures[t].hasTexture) ? 1 : 0;
            fileStream.write(reinterpret_cast<const char*>(&h), sizeof(uint8_t));

            // useForcedColor
            uint8_t f = (model.meshes()[m]._textures[t].useForcedColor) ? 1 : 0;
            fileStream.write(reinterpret_cast<const char*>(&f), sizeof(uint8_t));

            // color
            fileStream.write(
                reinterpret_cast<const char*>(&model.meshes()[m]._textures[t].color.r),
                sizeof(float)
            );
            fileStream.write(
                reinterpret_cast<const char*>(&model.meshes()[m]._textures[t].color.g),
                sizeof(float)
            );
            fileStream.write(
                reinterpret_cast<const char*>(&model.meshes()[m]._textures[t].color.b),
                sizeof(float)
            );

            // texture
            if (model.meshes()[m]._textures[t].hasTexture) {
                // Search the textureStorage to find which entry this texture points to
                bool wasFound = false;
                for (unsigned int te = 0; te < model.textureStorage().size(); ++te) {
                    if (model.textureStorage()[te].name ==
                        model.meshes()[m]._textures[t].texture->name())
                    {
                        uint32_t index = static_cast<uint32_t>(te);
                        fileStream.write(
                            reinterpret_cast<const char*>(&index),
                            sizeof(uint32_t)
                        );
                        wasFound = true;
                        break;
                    }
                }

                if (!wasFound) {
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
