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
#include <glm/gtc/type_ptr.hpp>

namespace {
    constexpr const char* _loggerCat = "ModelGeometry";
    constexpr const int8_t CurrentCacheVersion = 5;
    constexpr const int FormatStringSize = 4;

    ghoul::opengl::Texture::Format stringToFormat(std::string_view format) {
        using Format = ghoul::opengl::Texture::Format;
        if (format == "Red ") { return Format::Red; }
        else if (format == "RG  ") { return Format::RG; }
        else if (format == "RGB ") { return Format::RGB; }
        else if (format == "BGR ") { return Format::BGR; }
        else if (format == "RGBA") { return Format::RGBA; }
        else if (format == "BGRA") { return Format::BGRA; }
        else if (format == "Dept") { return Format::DepthComponent; }
        else { throw ghoul::MissingCaseException(); }
    }

    std::string formatToString(ghoul::opengl::Texture::Format format) {
        switch (format) {
            case ghoul::opengl::Texture::Format::Red: return "Red ";
            case ghoul::opengl::Texture::Format::RG: return "RG  ";
            case ghoul::opengl::Texture::Format::RGB: return "RGB ";
            case ghoul::opengl::Texture::Format::BGR: return "BGR ";
            case ghoul::opengl::Texture::Format::RGBA: return "RGBA";
            case ghoul::opengl::Texture::Format::BGRA: return "BGRA";
            case ghoul::opengl::Texture::Format::DepthComponent: return "Dept";
            default: throw ghoul::MissingCaseException();
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
        else { throw ghoul::MissingCaseException(); }
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
            default: throw ghoul::MissingCaseException();
        }
    }
} // namespace

namespace ghoul::modelgeometry {

ModelGeometry::ModelCacheException::ModelCacheException(std::string file,
                                                        std::string msg)
    : RuntimeError(fmt::format("Error: '{}' with cache file: '{}'", msg, file))
    , filename(std::move(file))
    , errorMessage(std::move(msg))
{}

ModelGeometry::ModelGeometry(std::vector<io::ModelNode> nodes,
                             std::vector<TextureEntry> textureStorage,
                             std::unique_ptr<io::ModelAnimation> animation)
    : _nodes(std::move(nodes))
    , _textureStorage(std::move(textureStorage))
    , _animation(std::move(animation))
{}

std::unique_ptr<modelgeometry::ModelGeometry> ModelGeometry::loadCacheFile(
                                                           const std::string& cachedFile)
{
    std::ifstream fileStream(cachedFile, std::ifstream::binary);
    if (!fileStream.good()) {
        throw ModelCacheException(cachedFile, "Could not open file");
    }

    // Check the caching version
    int8_t version = 0;
    fileStream.read(reinterpret_cast<char*>(&version), sizeof(int8_t));
    if (version != CurrentCacheVersion) {
        throw ModelCacheException(
            cachedFile,
            "The format of the cached file has changed"
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

    for (int32_t te = 0; te < nTextureEntries; ++te) {
        modelgeometry::ModelGeometry::TextureEntry textureEntry;

        // Name
        int32_t nameSize = 0;
        fileStream.read(reinterpret_cast<char*>(&nameSize), sizeof(int32_t));
        if (nameSize == 0) {
            throw ModelCacheException(cachedFile, "No texture name was loaded");
        }
        textureEntry.name.resize(nameSize);
        fileStream.read(textureEntry.name.data(), nameSize);

        // Texture
        // dimensions
        std::array<int32_t, 3> dimensionStorage;
        fileStream.read(
            reinterpret_cast<char*>(dimensionStorage.data()),
            3 * sizeof(int32_t)
        );
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
            throw ModelCacheException(cachedFile, "No texture size was loaded");
        }
        std::byte* data = new std::byte[textureSize];
        fileStream.read(reinterpret_cast<char*>(data), textureSize);

        textureEntry.texture = std::make_unique<opengl::Texture>(
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

    // Read how many nodes to read
    int32_t nNodes = 0;
    fileStream.read(reinterpret_cast<char*>(&nNodes), sizeof(int32_t));
    if (nNodes == 0) {
        throw ModelCacheException(cachedFile, "No nodes were loaded");
    }

    // Nodes
    std::vector<io::ModelNode> nodeArray;
    nodeArray.reserve(nNodes);
    for (int32_t n = 0; n < nNodes; ++n) {
        // Read how many meshes to read
        int32_t nMeshes = 0;
        fileStream.read(reinterpret_cast<char*>(&nMeshes), sizeof(int32_t));

        // Meshes
        std::vector<io::ModelMesh> meshArray;
        meshArray.reserve(nMeshes);
        for (int32_t m = 0; m < nMeshes; ++m) {
            // Vertices
            int32_t nVertices = 0;
            fileStream.read(reinterpret_cast<char*>(&nVertices), sizeof(int32_t));
            if (nVertices == 0) {
                throw ModelCacheException(cachedFile, "No vertices were loaded");
            }
            std::vector<io::ModelMesh::Vertex> vertexArray;
            vertexArray.reserve(nVertices);

            for (int32_t v = 0; v < nVertices; ++v) {
                io::ModelMesh::Vertex vertex;
                fileStream.read(
                    reinterpret_cast<char*>(&vertex),
                    sizeof(io::ModelMesh::Vertex)
                );
                vertexArray.push_back(std::move(vertex));
            }

            // Indices
            int32_t nIndices = 0;
            fileStream.read(reinterpret_cast<char*>(&nIndices), sizeof(int32_t));
            if (nIndices == 0) {
                throw ModelCacheException(cachedFile, "No indices were loaded");
            }
            std::vector<unsigned int> indexArray;
            indexArray.reserve(nIndices);

            for (int32_t i = 0; i < nIndices; ++i) {
                uint32_t index;
                fileStream.read(reinterpret_cast<char*>(&index), sizeof(uint32_t));
                indexArray.push_back(std::move(static_cast<unsigned int>(index)));
            }

            // Textures
            int32_t nTextures = 0;
            fileStream.read(reinterpret_cast<char*>(&nTextures), sizeof(int32_t));
            if (nTextures == 0) {
                throw ModelCacheException(cachedFile, "No textures were loaded");
            }
            std::vector<io::ModelMesh::Texture> textureArray;
            textureArray.reserve(nTextures);

            for (int32_t t = 0; t < nTextures; ++t) {
                io::ModelMesh::Texture texture;

                // type
                fileStream.read(reinterpret_cast<char*>(&texture.type), sizeof(uint8_t));

                // hasTexture
                uint8_t h;
                fileStream.read(reinterpret_cast<char*>(&h), sizeof(uint8_t));
                texture.hasTexture = h == 1;

                // useForcedColor
                uint8_t f;
                fileStream.read(reinterpret_cast<char*>(&f), sizeof(uint8_t));
                texture.useForcedColor = f == 1;

                // color
                fileStream.read(reinterpret_cast<char*>(&texture.color.r), sizeof(float));
                fileStream.read(reinterpret_cast<char*>(&texture.color.g), sizeof(float));
                fileStream.read(reinterpret_cast<char*>(&texture.color.b), sizeof(float));

                // texture
                if (texture.hasTexture) {
                    // Read which index in the textureStorageArray that this texture
                    // should point to
                    uint32_t index;
                    fileStream.read(reinterpret_cast<char*>(&index), sizeof(uint32_t));

                    if (index >= textureStorageArray.size()) {
                        throw ModelCacheException(
                            cachedFile,
                            "Texture index is outside of textureStorage"
                        );
                    }

                    texture.texture = textureStorageArray[index].texture.get();
                }
                textureArray.push_back(std::move(texture));
            }

            // Make mesh
            meshArray.push_back(io::ModelMesh(
                std::move(vertexArray),
                std::move(indexArray),
                std::move(textureArray)
            ));
        }

        // Transform
        glm::mat4x4 transform;
        GLfloat rawTransform[16];
        fileStream.read(reinterpret_cast<char*>(rawTransform), 16 * sizeof(GLfloat));
        transform = glm::make_mat4(rawTransform);

        // AnimationTransform
        glm::mat4x4 animationTransform;
        GLfloat rawAnimTransform[16];
        fileStream.read(reinterpret_cast<char*>(&rawAnimTransform), 16 * sizeof(GLfloat));
        animationTransform = glm::make_mat4(rawAnimTransform);

        // Parent
        int32_t parent = 0;
        fileStream.read(reinterpret_cast<char*>(&parent), sizeof(int32_t));

        // Read how many children to read
        int32_t nChildren = 0;
        fileStream.read(reinterpret_cast<char*>(&nChildren), sizeof(int32_t));

        // Children
        std::vector<int> childrenArray;
        nodeArray.reserve(nChildren);
        for (int32_t c = 0; c < nChildren; ++c) {
            int32_t child;
            fileStream.read(reinterpret_cast<char*>(&child), sizeof(int32_t));
            childrenArray.push_back(child);
        }

        // HasAnimation
        uint8_t a;
        fileStream.read(reinterpret_cast<char*>(&a), sizeof(uint8_t));
        bool hasAnimation = (a == 1);

        // Create Node
        io::ModelNode node = io::ModelNode(std::move(transform), std::move(meshArray));
        node.setChildren(std::move(childrenArray));
        node.setParent(parent);
        if (hasAnimation) {
            node.setAnimation(animationTransform);
        }

        nodeArray.push_back(std::move(node));
    }

    // Animation
    uint8_t anim;
    fileStream.read(reinterpret_cast<char*>(&anim), sizeof(uint8_t));
    bool hasAnimation = anim == 1;

    if (hasAnimation) {
        // Name
        uint8_t nameSize = 0;
        fileStream.read(reinterpret_cast<char*>(&nameSize), sizeof(uint8_t));
        std::string name;
        fileStream.read(name.data(), nameSize);

        // Duration
        double duration;
        fileStream.read(reinterpret_cast<char*>(&duration), sizeof(double));

        // TimeScale
        float timeScale;
        fileStream.read(reinterpret_cast<char*>(&timeScale), sizeof(float));

        // Read how many NodeAnimations to read
        int32_t nNodeAnimations = 0;
        fileStream.read(reinterpret_cast<char*>(&nNodeAnimations), sizeof(int32_t));
        if (nNodeAnimations == 0) {
            throw ModelCacheException(cachedFile, "No node animations were loaded");
        }

        // NodeAnimations
        std::unique_ptr<io::ModelAnimation> animation =
            std::make_unique<io::ModelAnimation>(io::ModelAnimation(name, duration));
        animation->nodeAnimations().reserve(nNodeAnimations);
        animation->setTimeScale(timeScale);
        for (int32_t na = 0; na < nNodeAnimations; ++na) {
            io::ModelAnimation::NodeAnimation nodeAnimation;

            // Node index
            int32_t nodeIndex;
            fileStream.read(reinterpret_cast<char*>(&nodeIndex), sizeof(int32_t));
            nodeAnimation.node = nodeIndex;

            // Positions
            uint8_t nPos;
            fileStream.read(reinterpret_cast<char*>(&nPos), sizeof(uint8_t));
            nodeAnimation.positions.reserve(nPos);
            for (uint8_t p = 0; p < nPos; ++p) {
                io::ModelAnimation::PositionKeyframe posKeyframe;

                // Position
                glm::vec3 pos;
                fileStream.read(reinterpret_cast<char*>(&pos.x), sizeof(float));
                fileStream.read(reinterpret_cast<char*>(&pos.y), sizeof(float));
                fileStream.read(reinterpret_cast<char*>(&pos.z), sizeof(float));
                posKeyframe.position = pos;

                // Time
                double time;
                fileStream.read(reinterpret_cast<char*>(&time), sizeof(double));
                posKeyframe.time = time;

                nodeAnimation.positions.push_back(std::move(posKeyframe));
            }

            // Rotations
            uint8_t nRot;
            fileStream.read(reinterpret_cast<char*>(&nRot), sizeof(uint8_t));
            nodeAnimation.rotations.reserve(nRot);
            for (uint8_t p = 0; p < nRot; ++p) {
                io::ModelAnimation::RotationKeyframe rotKeyframe;

                // Rotation
                float rotW, rotX, rotY, rotZ;
                fileStream.read(reinterpret_cast<char*>(&rotW), sizeof(float));
                fileStream.read(reinterpret_cast<char*>(&rotX), sizeof(float));
                fileStream.read(reinterpret_cast<char*>(&rotY), sizeof(float));
                fileStream.read(reinterpret_cast<char*>(&rotZ), sizeof(float));
                rotKeyframe.rotation = glm::quat(rotW, rotX, rotY, rotZ);

                // Time
                double time;
                fileStream.read(reinterpret_cast<char*>(&time), sizeof(double));
                rotKeyframe.time = time;

                nodeAnimation.rotations.push_back(std::move(rotKeyframe));
            }

            // Scales
            uint8_t nScale;
            fileStream.read(reinterpret_cast<char*>(&nScale), sizeof(uint8_t));
            nodeAnimation.scales.reserve(nScale);
            for (uint8_t p = 0; p < nScale; ++p) {
                io::ModelAnimation::ScaleKeyframe scaleKeyframe;

                // Scale
                glm::vec3 scale;
                fileStream.read(reinterpret_cast<char*>(&scale.x), sizeof(float));
                fileStream.read(reinterpret_cast<char*>(&scale.y), sizeof(float));
                fileStream.read(reinterpret_cast<char*>(&scale.z), sizeof(float));
                scaleKeyframe.scale = scale;

                // Time
                double time;
                fileStream.read(reinterpret_cast<char*>(&time), sizeof(double));
                scaleKeyframe.time = time;

                nodeAnimation.scales.push_back(std::move(scaleKeyframe));
            }

            animation->nodeAnimations().push_back(nodeAnimation);
        }

        // Create the ModelGeometry
        return std::make_unique<modelgeometry::ModelGeometry>(
            std::move(nodeArray),
            std::move(textureStorageArray),
            std::move(animation)
        );
    }
    else {
        // Create the ModelGeometry
        return std::make_unique<modelgeometry::ModelGeometry>(
            std::move(nodeArray),
            std::move(textureStorageArray),
            nullptr
        );
    }
}

bool ModelGeometry::saveToCacheFile(const std::string& cachedFile) const {
    std::ofstream fileStream(cachedFile, std::ofstream::binary);
    if (!fileStream.good()) {
        throw ModelCacheException( cachedFile, "Could not open file");
    }

    // Write which version of caching that is used
    fileStream.write(
        reinterpret_cast<const char*>(&CurrentCacheVersion),
        sizeof(int8_t)
    );

    // First cache the textureStorage
    int32_t nTextureEntries = static_cast<int32_t>(_textureStorage.size());
    if (nTextureEntries == 0) {
        LINFO("No TextureEntries were loaded while saving cache");
    }
    fileStream.write(reinterpret_cast<const char*>(&nTextureEntries), sizeof(int32_t));

    for (int32_t te = 0; te < nTextureEntries; ++te) {
        // Name
        int32_t nameSize = static_cast<int32_t>(
            _textureStorage[te].name.size() * sizeof(char)
        );
        if (nameSize == 0) {
            throw ModelCacheException(cachedFile, "No texture name was loaded");
        }
        fileStream.write(reinterpret_cast<const char*>(&nameSize), sizeof(int32_t));
        fileStream.write(_textureStorage[te].name.data(), nameSize);

        // Texture
        // dimensions
        std::array<int32_t, 3> dimensionStorage;
        dimensionStorage[0] = _textureStorage[te].texture->dimensions().x;
        dimensionStorage[1] = _textureStorage[te].texture->dimensions().y;
        dimensionStorage[2] = _textureStorage[te].texture->dimensions().z;

        fileStream.write(
            reinterpret_cast<const char*>(dimensionStorage.data()),
            3 * sizeof(int32_t)
        );

        // format
        std::string format = formatToString(_textureStorage[te].texture->format());
        fileStream.write(format.data(), FormatStringSize * sizeof(char));

        // internal format
        uint32_t internalFormat = static_cast<uint32_t>(
            _textureStorage[te].texture->internalFormat()
        );
        fileStream.write(
            reinterpret_cast<const char*>(&internalFormat),
            sizeof(uint32_t)
        );

        // data type
        std::string dataType =
            dataTypeToString(_textureStorage[te].texture->dataType());
        fileStream.write(dataType.data(), FormatStringSize * sizeof(char));

        // data
        _textureStorage[te].texture->downloadTexture();
        int32_t pixelSize = static_cast<int32_t>(
            _textureStorage[te].texture->expectedPixelDataSize()
        );
        if (pixelSize == 0) {
            throw ModelCacheException(cachedFile, "No texture size was loaded");
        }
        fileStream.write(reinterpret_cast<const char*>(&pixelSize), sizeof(int32_t));

        const void* data = _textureStorage[te].texture->pixelData();
        fileStream.write(reinterpret_cast<const char*>(data), pixelSize);
    }

    // Write how many nodes are to be written
    int32_t nNodes = static_cast<int32_t>(_nodes.size());
    if (nNodes == 0) {
        throw ModelCacheException(cachedFile, "No nodes were loaded");
    }
    fileStream.write(reinterpret_cast<const char*>(&nNodes), sizeof(int32_t));

    // Nodes
    for (const io::ModelNode& node : _nodes) {
        // Write how many meshes are to be written
        int32_t nMeshes = static_cast<int32_t>(node.meshes().size());
        fileStream.write(reinterpret_cast<const char*>(&nMeshes), sizeof(int32_t));

        // Meshes
        for (const io::ModelMesh& mesh : node.meshes()) {
            // Vertices
            int32_t nVertices = static_cast<int32_t>(mesh.vertices().size());
            if (nVertices == 0) {
                throw ModelCacheException(cachedFile, "No vertices were loaded");
            }
            fileStream.write(reinterpret_cast<const char*>(&nVertices), sizeof(int32_t));

            for (int32_t v = 0; v < nVertices; ++v) {
                fileStream.write(
                    reinterpret_cast<const char*>(&mesh.vertices()[v]),
                    sizeof(io::ModelMesh::Vertex)
                );
            }

            // Indices
            int32_t nIndices = static_cast<int32_t>(mesh.indices().size());
            if (nIndices == 0) {
                throw ModelCacheException(cachedFile, "No indices were loaded");
            }
            fileStream.write(reinterpret_cast<const char*>(&nIndices), sizeof(int32_t));

            for (int32_t i = 0; i < nIndices; ++i) {
                uint32_t index = static_cast<uint32_t>(mesh.indices()[i]);
                fileStream.write(reinterpret_cast<const char*>(&index), sizeof(uint32_t));
            }

            // Textures
            int32_t nTextures = static_cast<int32_t>(mesh.textures().size());
            if (nTextures == 0) {
                throw ModelCacheException(cachedFile, "No textures were loaded");
            }
            fileStream.write(reinterpret_cast<const char*>(&nTextures), sizeof(int32_t));

            for (int32_t t = 0; t < nTextures; ++t) {
                // type
                fileStream.write(
                    reinterpret_cast<const char*>(&mesh.textures()[t].type),
                    sizeof(uint8_t)
                );

                // hasTexture
                uint8_t h = (mesh.textures()[t].hasTexture) ? 1 : 0;
                fileStream.write(reinterpret_cast<const char*>(&h), sizeof(uint8_t));

                // useForcedColor
                uint8_t f = (mesh.textures()[t].useForcedColor) ? 1 : 0;
                fileStream.write(reinterpret_cast<const char*>(&f), sizeof(uint8_t));

                // color
                fileStream.write(
                    reinterpret_cast<const char*>(&mesh.textures()[t].color.r),
                    sizeof(float)
                );
                fileStream.write(
                    reinterpret_cast<const char*>(&mesh.textures()[t].color.g),
                    sizeof(float)
                );
                fileStream.write(
                    reinterpret_cast<const char*>(&mesh.textures()[t].color.b),
                    sizeof(float)
                );

                // texture
                if (mesh.textures()[t].hasTexture) {
                    // Search the textureStorage to find the texture entry
                    bool wasFound = false;
                    for (size_t te = 0; te < _textureStorage.size(); ++te) {
                        if (_textureStorage[te].name ==
                            mesh.textures()[t].texture->name())
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
                        throw ModelCacheException(
                            cachedFile,
                            "Could not find texture in textureStorage"
                        );
                    }
                }
            }
        }

        // Transform
        glm::mat4x4 transform = node.transform();
        fileStream.write(reinterpret_cast<const char*>(&transform), 16 * sizeof(GLfloat));

        // AnimationTransform
        glm::mat4x4 animationTransform = node.animationTransform();
        fileStream.write(
            reinterpret_cast<const char*>(&animationTransform),
            16 * sizeof(GLfloat)
        );

        // Parent
        int32_t parent = static_cast<int32_t>(node.parent());
        fileStream.write(reinterpret_cast<const char*>(&parent), sizeof(int32_t));

        // Write how many children are to be written
        int32_t nChildren = static_cast<int32_t>(node.children().size());
        fileStream.write(reinterpret_cast<const char*>(&nChildren), sizeof(int32_t));

        // Children
        for (int32_t child : node.children()) {
            fileStream.write(reinterpret_cast<const char*>(&child), sizeof(int32_t));
        }

        // HasAnimation
        uint8_t a = node.hasAnimation() ? 1 : 0;
        fileStream.write(reinterpret_cast<const char*>(&a), sizeof(uint8_t));
    }

    // Animation
    uint8_t a = _animation != nullptr ? 1 : 0;
    fileStream.write(reinterpret_cast<const char*>(&a), sizeof(uint8_t));

    if (_animation != nullptr) {
        // Name
        uint8_t nameSize = _animation->name().size();
        fileStream.write(reinterpret_cast<const char*>(&nameSize), sizeof(uint8_t));
        fileStream.write(
            reinterpret_cast<const char*>(_animation->name().data()),
            nameSize * sizeof(char)
        );

        // Duration
        double duration = _animation->duration();
        fileStream.write(reinterpret_cast<const char*>(&duration), sizeof(double));

        // TimeScale
        float timeScale = _animation->timeScale();
        fileStream.write(reinterpret_cast<const char*>(&timeScale), sizeof(float));

        // Write how many NodeAnimations are to be written
        int32_t nAnimations = static_cast<int32_t>(_animation->nodeAnimations().size());
        if (nAnimations == 0) {
            throw ModelCacheException(cachedFile, "No node animations were loaded");
        }
        fileStream.write(reinterpret_cast<const char*>(&nAnimations), sizeof(int32_t));

        // NodeAnimations
        for (const io::ModelAnimation::NodeAnimation& nodeAnimation :
            _animation->nodeAnimations())
        {
            // Node index
            int32_t nodeIndex = static_cast<int32_t>(nodeAnimation.node);
            fileStream.write(reinterpret_cast<const char*>(&nodeIndex), sizeof(int32_t));

            // Positions
            uint8_t nPos = nodeAnimation.positions.size();
            fileStream.write(reinterpret_cast<const char*>(&nPos), sizeof(uint8_t));
            for (const io::ModelAnimation::PositionKeyframe& posKeyframe :
                nodeAnimation.positions)
            {
                // Position
                fileStream.write(
                    reinterpret_cast<const char*>(&posKeyframe.position.x),
                    sizeof(float)
                );
                fileStream.write(
                    reinterpret_cast<const char*>(&posKeyframe.position.y),
                    sizeof(float)
                );
                fileStream.write(
                    reinterpret_cast<const char*>(&posKeyframe.position.z),
                    sizeof(float)
                );

                // Time
                fileStream.write(
                    reinterpret_cast<const char*>(&posKeyframe.time),
                    sizeof(double)
                );
            }

            // Rotations
            uint8_t nRot = nodeAnimation.rotations.size();
            fileStream.write(reinterpret_cast<const char*>(&nRot), sizeof(uint8_t));
            for (const io::ModelAnimation::RotationKeyframe& rotKeyframe :
                nodeAnimation.rotations)
            {
                // Rotation
                fileStream.write(
                    reinterpret_cast<const char*>(&rotKeyframe.rotation.w),
                    sizeof(float)
                );
                fileStream.write(
                    reinterpret_cast<const char*>(&rotKeyframe.rotation.x),
                    sizeof(float)
                );
                fileStream.write(
                    reinterpret_cast<const char*>(&rotKeyframe.rotation.y),
                    sizeof(float)
                );
                fileStream.write(
                    reinterpret_cast<const char*>(&rotKeyframe.rotation.z),
                    sizeof(float)
                );

                // Time
                fileStream.write(
                    reinterpret_cast<const char*>(&rotKeyframe.time),
                    sizeof(double)
                );
            }

            // Scales
            uint8_t nScale = nodeAnimation.scales.size();
            fileStream.write(reinterpret_cast<const char*>(&nScale), sizeof(uint8_t));
            for (const io::ModelAnimation::ScaleKeyframe& scaleKeyframe :
                nodeAnimation.scales)
            {
                // Scale
                fileStream.write(
                    reinterpret_cast<const char*>(&scaleKeyframe.scale.x),
                    sizeof(float)
                );
                fileStream.write(
                    reinterpret_cast<const char*>(&scaleKeyframe.scale.y),
                    sizeof(float)
                );
                fileStream.write(
                    reinterpret_cast<const char*>(&scaleKeyframe.scale.z),
                    sizeof(float)
                );

                // Time
                fileStream.write(
                    reinterpret_cast<const char*>(&scaleKeyframe.time),
                    sizeof(double)
                );
            }
        }
    }

    return fileStream.good();
}

double ModelGeometry::boundingRadius() const {
    return _boundingRadius;
}

void calculateBoundingRadiusRecursive(const std::vector<io::ModelNode>& nodes,
                                      const io::ModelNode* node,
                                      glm::mat4x4& parentTransform,
                                      float& maximumDistanceSquared)
{
    if (!node) {
        LERROR("Cannot calculate bounding radius for empty node");
        return;
    }

    // NOTE: The bounding radius will not change along with an animation
    glm::mat4x4 globalTransform = parentTransform * node->transform();

    for (const io::ModelMesh& mesh : node->meshes()) {
        float d = mesh.calculateBoundingRadius(globalTransform);
        maximumDistanceSquared = std::max(d, maximumDistanceSquared);
    }

    for (int child : node->children()) {
        calculateBoundingRadiusRecursive(
            nodes,
            &nodes[child],
            globalTransform,
            maximumDistanceSquared
        );
    }
}

void ModelGeometry::calculateBoundingRadius() {
    if (_nodes.empty()) {
        LERROR("Cannot calculate bounding radius for empty geometry");
        return;
    }

    glm::mat4x4 parentTransform;
    float maximumDistanceSquared = 0.f;
    calculateBoundingRadiusRecursive(
        _nodes,
        _nodes.data(),
        parentTransform,
        maximumDistanceSquared
    );

    _boundingRadius = std::sqrt(maximumDistanceSquared);
}

bool ModelGeometry::hasAnimation() const {
    return _animation != nullptr;
}

double ModelGeometry::animationDuration() const {
    if (_animation == nullptr) {
        LERROR("Model does not have any animation");
        return -1.0;
    }

    return _animation->duration();
}

std::vector<io::ModelNode>& ModelGeometry::nodes() {
    return _nodes;
}

const std::vector<io::ModelNode>& ModelGeometry::nodes() const {
    return _nodes;
}

std::vector<ModelGeometry::TextureEntry>& ModelGeometry::textureStorage() {
    return _textureStorage;
}

const std::vector<ModelGeometry::TextureEntry>& ModelGeometry::textureStorage() const {
    return _textureStorage;
}

void renderRecursive(const std::vector<io::ModelNode>& nodes, const io::ModelNode* node,
                     opengl::ProgramObject& program, glm::mat4x4& parentTransform,
                     bool isTexturedModel)
{
    if (!node) {
        LERROR("Cannot render empty node");
        return;
    }

    glm::mat4x4 globalTransform;
    if (node->hasAnimation()) {
        // Animation is given by Assimp in absolute format
        // i.e. animation replaces old transform
        globalTransform = parentTransform * node->animationTransform();
    }
    else {
        globalTransform = parentTransform * node->transform();
    }

    for (const io::ModelMesh& mesh : node->meshes()) {
        mesh.render(program, globalTransform, isTexturedModel);
    }

    for (int child : node->children()) {
        renderRecursive(nodes, &nodes[child], program, globalTransform, isTexturedModel);
    }
}

void ModelGeometry::render(opengl::ProgramObject& program, bool isTexturedModel) const {
    if (_nodes.empty()) {
        LERROR("Cannot render empty geometry");
        return;
    }

    glm::mat4x4 parentTransform;
    renderRecursive(_nodes, _nodes.data(), program, parentTransform, isTexturedModel);
}

void ModelGeometry::update(double now) {
    if (_animation == nullptr) {
        LERROR("Cannot update empty animation");
        return;
    }

    _animation->animate(_nodes, now, _animationEnabled);
}

void ModelGeometry::setTimeScale(float timeScale) {
    if (_animation == nullptr) {
        LERROR("Cannot set time scale of empty animation");
        return;
    }
    _animation->setTimeScale(timeScale);
}

void ModelGeometry::enableAnimation(bool value) {
    _animationEnabled = value;

    if (!value) {
        _animation->reset(_nodes);
    }
}

void ModelGeometry::initialize() {
    ZoneScoped

    for (io::ModelNode& node : _nodes) {
        for (io::ModelMesh& mesh : node.meshes()) {
            mesh.initialize();
        }
    }

    calculateBoundingRadius();
}

void ModelGeometry::deinitialize() {
    for (io::ModelNode& node : _nodes) {
        for (io::ModelMesh& mesh : node.meshes()) {
            mesh.deinitialize();
        }
    }
}

}  // namespace openspace::modelgeometry
