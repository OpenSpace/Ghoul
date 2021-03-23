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

#include <ghoul/io/model/modelanimation.h>

#include <glm/gtx/quaternion.hpp>

namespace ghoul::io {

ModelAnimation::ModelAnimation(std::string name, double duration)
    : _name(name)
    , _duration(duration)
{}

void ModelAnimation::setTimeScale(float timeScale) {
    _timeScale = timeScale;
    _duration = _duration * _timeScale;
}

void ModelAnimation::animate(std::vector<ModelNode>& nodes, double now, bool enabled) {
    // Animation out of scope or disabled
    if (!enabled || now > _duration || now < 0) {
        // Reset animation
        for (const io::ModelAnimation::NodeAnimation& nodeAnimation :
            _nodeAnimations)
        {
            nodes[nodeAnimation.node].setAnimation(nodes[nodeAnimation.node].transform());
        }

        return;
    }

    // Find keyframes
    for (const io::ModelAnimation::NodeAnimation& nodeAnimation :
        _nodeAnimations)
    {
        // Position
        glm::vec3 currPos;
        if (nodeAnimation.positions.size() > 1) {
            double prevPosTime = -DBL_MAX;
            glm::vec3 prevPos;
            double nextPosTime = DBL_MAX;
            glm::vec3 nextPos;
            bool interpolate = true;

            for (const io::ModelAnimation::PositionKeyframe& pos : nodeAnimation.positions) {
                double diff = (pos.time * _timeScale) - now;

                // Exact on a keyframe
                if (diff == 0.0) {
                    currPos = pos.position;
                    interpolate = false;
                }
                // Prev keyframe
                else if (diff < 0 && diff >(prevPosTime - now)) {
                    prevPosTime = pos.time * _timeScale;
                    prevPos = pos.position;
                }
                // next keyframe
                else if (diff > 0 && diff < (nextPosTime - now)) {
                    nextPosTime = pos.time * _timeScale;
                    nextPos = pos.position;
                }
            }

            if (interpolate) {
                double blend = (now - prevPosTime) / (nextPosTime - prevPosTime);
                currPos = glm::mix(prevPos, nextPos, static_cast<float>(blend));
            }
        }
        else {
            currPos = nodeAnimation.positions[0].position;
        }

        // Rotation
        glm::quat currRot;
        if (nodeAnimation.rotations.size() > 1) {
            double prevRotTime = -DBL_MAX;
            glm::quat prevRot;
            double nextRotTime = DBL_MAX;
            glm::quat nextRot;
            bool interpolate = true;

            for (const io::ModelAnimation::RotationKeyframe& rot : nodeAnimation.rotations) {
                double diff = (rot.time * _timeScale) - now;

                // Exact on a keyframe
                if (diff == 0.0) {
                    currRot = rot.rotation;
                    interpolate = false;
                }
                // Prev keyframe
                else if (diff < 0 && diff >(prevRotTime - now)) {
                    prevRotTime = rot.time * _timeScale;
                    prevRot = rot.rotation;
                }
                // next keyframe
                else if (diff > 0 && diff < (nextRotTime - now)) {
                    nextRotTime = rot.time * _timeScale;
                    nextRot = rot.rotation;
                }
            }

            if (interpolate) {
                double blend = (now - prevRotTime) / (nextRotTime - prevRotTime);
                currRot = glm::slerp(prevRot, nextRot, static_cast<float>(blend));
            }
        }
        else {
            currRot = nodeAnimation.rotations[0].rotation;
        }

        // Scale
        glm::vec3 currScale;
        if (nodeAnimation.scales.size() > 1) {
            double prevScaleTime = -DBL_MAX;
            glm::vec3 prevScale;
            double nextScaleTime = DBL_MAX;
            glm::vec3 nextScale;
            bool interpolate = true;

            for (const io::ModelAnimation::ScaleKeyframe& scale : nodeAnimation.scales) {
                double diff = (scale.time * _timeScale) - now;

                // Exact on a keyframe
                if (diff == 0.0) {
                    currScale = scale.scale;
                    interpolate = false;
                }
                // Prev keyframe
                else if (diff < 0 && diff >(prevScaleTime - now)) {
                    prevScaleTime = scale.time * _timeScale;
                    prevScale = scale.scale;
                }
                // next keyframe
                else if (diff > 0 && diff < (nextScaleTime - now)) {
                    nextScaleTime = scale.time * _timeScale;
                    nextScale = scale.scale;
                }
            }

            if (interpolate) {
                double blend = (now - prevScaleTime) / (nextScaleTime - prevScaleTime);
                currScale = glm::mix(prevScale, nextScale, static_cast<float>(blend));
            }
        }
        else {
            currScale = nodeAnimation.scales[0].scale;
        }

        glm::mat4x4 animationTransform;
        animationTransform =
            glm::translate(glm::mat4(1.0), currPos) *
            glm::toMat4(currRot) *
            glm::scale(animationTransform, currScale);

        nodes[nodeAnimation.node].setAnimation(animationTransform);
    }
}

std::vector<ModelAnimation::NodeAnimation>& ModelAnimation::nodeAnimations() {
    return _nodeAnimations;
}

const std::vector<ModelAnimation::NodeAnimation>& ModelAnimation::nodeAnimations() const {
    return _nodeAnimations;
}

std::string ModelAnimation::name() const {
    return _name;
}

double ModelAnimation::duration() const {
    return _duration;
}

float ModelAnimation::timeScale() const {
    return _timeScale;
}

} // namespace ghoul::io