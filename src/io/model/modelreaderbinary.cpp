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

#include <ghoul/io/model/modelreaderbinary.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <fstream>

namespace {
    constexpr const char* _loggerCat = "ModelReaderBinary";
    constexpr const int8_t CurrentModelVersion = 6;
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

namespace ghoul::io {

std::unique_ptr<modelgeometry::ModelGeometry> ModelReaderBinary::loadModel(
                                                              const std::string& filename,
                                                                bool forceRenderInvisible,
                                                        bool notifyInvisibleDropped) const
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    std::ifstream fileStream(filename, std::ifstream::binary);
    if (!fileStream.good()) {
        throw ModelLoadException(filename, "Could not open file", this);
    }

    // Check the file format version
    int8_t version = 0;
    fileStream.read(reinterpret_cast<char*>(&version), sizeof(int8_t));
    if (version != CurrentModelVersion) {
        throw ModelLoadException(
            filename,
            "The format of the OS-model file has changed",
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

    for (int32_t te = 0; te < nTextureEntries; ++te) {
        modelgeometry::ModelGeometry::TextureEntry textureEntry;

        // Name
        int32_t nameSize = 0;
        fileStream.read(reinterpret_cast<char*>(&nameSize), sizeof(int32_t));
        if (nameSize == 0) {
            throw ModelLoadException(filename, "No texture name was loaded", this);
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
            throw ModelLoadException(filename, "No texture size was loaded", this);
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
        throw ModelLoadException(filename, "No nodes were loaded", this);
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
                throw ModelLoadException(filename, "No vertices were loaded", this);
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
                throw ModelLoadException(filename, "No indices were loaded", this);
            }
            std::vector<unsigned int> indexArray;
            indexArray.reserve(nIndices);

            for (int32_t i = 0; i < nIndices; ++i) {
                uint32_t index;
                fileStream.read(reinterpret_cast<char*>(&index), sizeof(uint32_t));
                indexArray.push_back(index);
            }

            // IsInvisible
            uint8_t inv;
            fileStream.read(reinterpret_cast<char*>(&inv), sizeof(uint8_t));
            bool isInvisible = (inv == 1);

            // Textures
            int32_t nTextures = 0;
            fileStream.read(reinterpret_cast<char*>(&nTextures), sizeof(int32_t));
            if (nTextures == 0 && !isInvisible) {
                throw ModelLoadException(filename, "No textures were loaded", this);
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
                texture.hasTexture = (h == 1);

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
                        throw ModelLoadException(
                            filename,
                            "Texture index is outside of textureStorage",
                            this
                        );
                    }

                    texture.texture = textureStorageArray[index].texture.get();
                }
                textureArray.push_back(std::move(texture));
            }

            // If mesh is invisible then check if it should be forced to render with
            // flashy colors and/or there should ba a notification
            if (isInvisible) {
                if (forceRenderInvisible) {
                    // Force invisible mesh to render with flashy colors
                    io::ModelMesh::Texture texture;
                    io::ModelMesh::generateDebugTexture(texture);
                    textureArray.push_back(std::move(texture));
                }
                else if (notifyInvisibleDropped) {
                    LINFO("An invisible mesh has been dropped");
                }
            }

            // Make mesh
            meshArray.push_back(io::ModelMesh(
                std::move(vertexArray),
                std::move(indexArray),
                std::move(textureArray),
                isInvisible
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
            int child;
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
    bool hasAnimation = (anim == 1);

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
            throw ModelLoadException(filename, "No node animations were loaded", this);
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

bool ModelReaderBinary::needsCache() const {
    return false;
}

std::vector<std::string> ModelReaderBinary::supportedExtensions() const {
    return {
        "osmodel" // Custom OpenSpace model format
    };
}

} // namespace ghoul::io
