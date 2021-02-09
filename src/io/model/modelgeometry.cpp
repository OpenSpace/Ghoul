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

namespace {
    constexpr const char* _loggerCat = "ModelGeometry";
    constexpr const int8_t CurrentCacheVersion = 4;
    constexpr const int FormatStringSize = 4;
} // namespace

namespace ghoul::modelgeometry {

ModelGeometry::ModelCacheException::ModelCacheException(std::string file,
    std::string msg)
    : RuntimeError(fmt::format(
        "Error:'{}' with cache file:'{}'", msg, file
    ))
    , message(std::move(msg))
    , filename(std::move(file))
{}

ModelGeometry::ModelGeometry(std::vector<io::ModelMesh> meshes,
                             std::vector<TextureEntry> textureStorage)
    : _meshes(std::move(meshes))
    , _textureStorage(std::move(textureStorage))
{}

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

std::unique_ptr<modelgeometry::ModelGeometry> ModelGeometry::loadCacheFile(
    const std::string& cachedFile) {
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

    for (unsigned int te = 0; te < nTextureEntries; ++te) {
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
            throw ModelCacheException(cachedFile, "No texture size was loaded");
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
        throw ModelCacheException(cachedFile, "No meshes were loaded");
    }

    std::vector<io::ModelMesh> meshArray;
    meshArray.reserve(nMeshes);

    // Read the meshes in same order as they were written
    for (unsigned int m = 0; m < nMeshes; ++m) {
        // Vertices
        int32_t nVertices = 0;
        fileStream.read(reinterpret_cast<char*>(&nVertices), sizeof(int32_t));
        if (nVertices == 0) {
            throw ModelCacheException(cachedFile, "No vertices were loaded");
        }
        std::vector<io::ModelMesh::Vertex> vertexArray;
        vertexArray.reserve(nVertices);

        for (unsigned int v = 0; v < nVertices; ++v) {
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

        for (unsigned int i = 0; i < nIndices; ++i) {
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

        for (unsigned int t = 0; t < nTextures; ++t) {
            io::ModelMesh::Texture texture;

            // type
            fileStream.read(reinterpret_cast<char*>(&texture.type), sizeof(uint8_t));

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

    // Create the ModelGeometry
    return std::make_unique<modelgeometry::ModelGeometry>(
        std::move(meshArray),
        std::move(textureStorageArray)
    );
}

bool ModelGeometry::saveToCacheFile(const std::string& cachedFile) const
{
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
    int32_t nTextureEntries = _textureStorage.size();
    if (nTextureEntries == 0) {
        LINFO("No TextureEntries were loaded while saving cache");
    }
    fileStream.write(reinterpret_cast<const char*>(&nTextureEntries), sizeof(int32_t));

    for (unsigned int te = 0; te < nTextureEntries; ++te) {
        // Name
        int32_t nameSize = _textureStorage[te].name.size() * sizeof(char);
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
        std::string format =
            formatToString(_textureStorage[te].texture->format());
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
        int32_t pixelSize = _textureStorage[te].texture->expectedPixelDataSize();
        if (pixelSize == 0) {
            throw ModelCacheException(cachedFile, "No texture size was loaded");
        }
        fileStream.write(reinterpret_cast<const char*>(&pixelSize), sizeof(int32_t));

        const void* data = _textureStorage[te].texture->pixelData();
        fileStream.write(reinterpret_cast<const char*>(data), pixelSize);
    }

    // Write how many meshes are to be written
    int32_t nMeshes = _meshes.size();
    if (nMeshes == 0) {
        throw ModelCacheException(cachedFile, "No meshes were loaded");
    }
    fileStream.write(reinterpret_cast<const char*>(&nMeshes), sizeof(int32_t));

    // Meshes
    for (unsigned int m = 0; m < nMeshes; m++) {
        // Vertices
        int32_t nVertices = _meshes[m].vertices().size();
        if (nVertices == 0) {
            throw ModelCacheException(cachedFile, "No vertices were loaded");
        }
        fileStream.write(reinterpret_cast<const char*>(&nVertices), sizeof(int32_t));

        for (unsigned int v = 0; v < nVertices; ++v) {
            fileStream.write(
                reinterpret_cast<const char*>(&_meshes[m].vertices()[v]),
                sizeof(io::ModelMesh::Vertex)
            );
        }

        // Indices
        int32_t nIndices = _meshes[m].indices().size();
        if (nIndices == 0) {
            throw ModelCacheException(cachedFile, "No indices were loaded");
        }
        fileStream.write(reinterpret_cast<const char*>(&nIndices), sizeof(int32_t));

        for (unsigned int i = 0; i < nIndices; ++i) {
            uint32_t index = static_cast<uint32_t>(_meshes[m].indices()[i]);
            fileStream.write(reinterpret_cast<const char*>(&index), sizeof(uint32_t));
        }

        // Textures
        int32_t nTextures = _meshes[m].textures().size();
        if (nTextures == 0) {
            throw ModelCacheException(cachedFile, "No textures were loaded");
        }
        fileStream.write(reinterpret_cast<const char*>(&nTextures), sizeof(int32_t));

        for (unsigned int t = 0; t < nTextures; ++t) {
            // type
            fileStream.write(
                reinterpret_cast<const char*>(&_meshes[m].textures()[t].type),
                sizeof(uint8_t)
            );

            // hasTexture
            uint8_t h = (_meshes[m].textures()[t].hasTexture) ? 1 : 0;
            fileStream.write(reinterpret_cast<const char*>(&h), sizeof(uint8_t));

            // useForcedColor
            uint8_t f = (_meshes[m].textures()[t].useForcedColor) ? 1 : 0;
            fileStream.write(reinterpret_cast<const char*>(&f), sizeof(uint8_t));

            // color
            fileStream.write(
                reinterpret_cast<const char*>(&_meshes[m].textures()[t].color.r),
                sizeof(float)
            );
            fileStream.write(
                reinterpret_cast<const char*>(&_meshes[m].textures()[t].color.g),
                sizeof(float)
            );
            fileStream.write(
                reinterpret_cast<const char*>(&_meshes[m].textures()[t].color.b),
                sizeof(float)
            );

            // texture
            if (_meshes[m].textures()[t].hasTexture) {
                // Search the textureStorage to find which entry this texture points to
                bool wasFound = false;
                for (unsigned int te = 0; te < _textureStorage.size(); ++te) {
                    if (_textureStorage[te].name ==
                        _meshes[m].textures()[t].texture->name())
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

    return fileStream.good();
}

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
