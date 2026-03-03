/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2026                                                               *
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

#include <ghoul/io/camera/camerareader.h>

#include <ghoul/io/model/modelanimation.h>
#include <ghoul/io/model/modelnode.h>
#include <ghoul/io/model/modelgeometry.h>
#include <ghoul/misc/profiling.h>
#include <ghoul/logging/logmanager.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <format>

namespace {
    constexpr std::string_view _loggerCat = "CameraReader";

    void processCameraNodes(const aiNode& node, const aiScene& scene,
                            std::vector<ghoul::io::ModelNode>& nodes, int parent,
                            std::unique_ptr<ghoul::io::ModelAnimation>& modelAnimation)
    {
        using namespace ghoul::io;

        // Convert transform matrix of the node
        // Assimp stores matrixes in row major and glm stores matrixes in column major
        const glm::mat4 nodeTransform = glm::mat4(
            node.mTransformation.a1, node.mTransformation.b1,
            node.mTransformation.c1, node.mTransformation.d1,

            node.mTransformation.a2, node.mTransformation.b2,
            node.mTransformation.c2, node.mTransformation.d2,

            node.mTransformation.a3, node.mTransformation.b3,
            node.mTransformation.c3, node.mTransformation.d3,

            node.mTransformation.a4, node.mTransformation.b4,
            node.mTransformation.c4, node.mTransformation.d4
        );

        ModelNode modelNode(nodeTransform, std::vector<ModelMesh>());
        modelNode.setParent(parent);
        nodes.push_back(std::move(modelNode));
        const int newNode = static_cast<int>(nodes.size() - 1);
        if (parent != -1) {
            nodes[parent].addChild(newNode);
        }

        // Check animations
        if (scene.HasAnimations()) {
            for (unsigned int a = 0; a < scene.mNumAnimations; a++) {
                aiAnimation* animation = scene.mAnimations[a];
                if (modelAnimation->name() != animation->mName.C_Str()) {
                    continue;
                }

                for (unsigned int c = 0; c < animation->mNumChannels; c++) {
                    aiNodeAnim* nodeAnim = animation->mChannels[c];

                    if (nodeAnim->mNodeName == node.mName) {
                        ModelAnimation::NodeAnimation nodeAnimation;
                        nodeAnimation.node = newNode;

                        for (unsigned int p = 0; p < nodeAnim->mNumPositionKeys; p++) {
                            const aiVectorKey posKey = nodeAnim->mPositionKeys[p];

                            ModelAnimation::PositionKeyframe positionKf;
                            positionKf.time =
                                std::abs(animation->mTicksPerSecond) <
                                std::numeric_limits<double>::epsilon() ? posKey.mTime :
                                posKey.mTime / animation->mTicksPerSecond;
                            positionKf.position = glm::vec3(
                                posKey.mValue.x,
                                posKey.mValue.y,
                                posKey.mValue.z
                            );

                            nodeAnimation.positions.push_back(std::move(positionKf));
                        }

                        for (unsigned int r = 0; r < nodeAnim->mNumRotationKeys; r++) {
                            const aiQuatKey rotKey = nodeAnim->mRotationKeys[r];

                            ModelAnimation::RotationKeyframe rotationKf;
                            rotationKf.time =
                                std::abs(animation->mTicksPerSecond) <
                                std::numeric_limits<double>::epsilon() ? rotKey.mTime :
                                rotKey.mTime / animation->mTicksPerSecond;
                            rotationKf.rotation = glm::quat(
                                rotKey.mValue.w,
                                rotKey.mValue.x,
                                rotKey.mValue.y,
                                rotKey.mValue.z
                            );

                            nodeAnimation.rotations.push_back(std::move(rotationKf));
                        }

                        for (unsigned int s = 0; s < nodeAnim->mNumScalingKeys; s++) {
                            const aiVectorKey scaleKey = nodeAnim->mScalingKeys[s];

                            ModelAnimation::ScaleKeyframe scaleKeyframe;
                            scaleKeyframe.time =
                                std::abs(animation->mTicksPerSecond) <
                                std::numeric_limits<double>::epsilon() ? scaleKey.mTime :
                                scaleKey.mTime / animation->mTicksPerSecond;
                            scaleKeyframe.scale = glm::vec3(
                                scaleKey.mValue.x,
                                scaleKey.mValue.y,
                                scaleKey.mValue.z
                            );

                            nodeAnimation.scales.push_back(std::move(scaleKeyframe));
                        }

                        modelAnimation->nodeAnimations().push_back(
                            std::move(nodeAnimation)
                        );
                        break;
                    }
                }
            }
        }

        // After we've processed all of the meshes (if any) we then recursively
        // process each of the children nodes (if any)
        for (unsigned int i = 0; i < node.mNumChildren; i++) {
            processCameraNodes(
                *(node.mChildren[i]),
                scene,
                nodes,
                newNode,
                modelAnimation
            );
        }
    };
} // namespace

namespace ghoul::io {

std::pair<std::vector<ModelNode>, std::unique_ptr<ModelAnimation>>
CameraReader::loadCameraPath(const std::filesystem::path& filename)
{
    ZoneScoped;

    ghoul_assert(!filename.empty(), "Filename must not be empty");

    Assimp::Importer importer;

    Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE, aiDefaultLogStream_STDOUT);
    Assimp::DefaultLogger::get()->info(std::format("Loading camera path from '{}'",
        filename
    ));

    const aiScene* scene = importer.ReadFile(
        filename.string(),
        aiProcess_ValidateDataStructure
    );

    // The mFlags is set when no mesh is loaded,
    // https://github.com/assimp/assimp/blob/7e5a0acc48efc54d7aa7900c36cd63db1fbeec9b/
    // code/Blender/BlenderLoader.cpp#L411-L417
    if (!scene /* scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE */) {
        throw ghoul::RuntimeError(std::format(
            "Error loading camera path '{}', '{}'",
            filename, importer.GetErrorString()
        ));
    }

    std::unique_ptr<ModelAnimation> modelAnimation = nullptr;
    if (scene->HasAnimations()) {
        // @TODO (anden88 2026-02-16): These check were taken as is from the model loading
        // Do not support more than one animation
        if (scene->mNumAnimations > 1) {
            LWARNING(
                "Detected more than one animation but currently only one is supported"
            );
        }
        aiAnimation* animation = scene->mAnimations[0];

        // Do not support morph animation
        if (animation->mNumMorphMeshChannels > 0) {
            LWARNING(
                "Detected unsupported animation type: 'Morph', currently only keyframe "
                "animations are supported"
            );
        }
        // Do not support animation that replaces the mesh for every frame
        if (animation->mNumMeshChannels > 0) {
            LWARNING(
                "Detected unsupported animation type: 'Mesh', currently only keyframe "
                "animations are supported"
            );
        }
        // Only support keyframe animation
        if (animation->mNumChannels > 0) {
            modelAnimation = std::make_unique<ModelAnimation>(
                animation->mName.C_Str(),
                std::abs(animation->mTicksPerSecond) <
                std::numeric_limits<double>::epsilon() ? // Not all formats have this
                animation->mDuration :
                animation->mDuration / animation->mTicksPerSecond
            );
        }
    }

    // Get info from all models in the scene
    std::vector<ModelNode> nodeArray;

    processCameraNodes(
        *(scene->mRootNode),
        *scene,
        nodeArray,
        -1,
        modelAnimation
    );

    Assimp::DefaultLogger::kill();

    return std::make_pair(std::move(nodeArray), std::move(modelAnimation));
}

} // namespace ghoul::io
