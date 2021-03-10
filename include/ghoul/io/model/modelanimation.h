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

#ifndef __GHOUL___MODELANIMATION___H__
#define __GHOUL___MODELANIMATION___H__

#include <ghoul/glm.h>
#include <vector>

namespace ghoul::io {

class ModelAnimation {
public:
    struct PositionKeyframe {
        glm::vec3 position;
        double time;
    };

    struct RotationKeyframe {
        glm::quat rotation;
        double time;
    };

    struct ScaleKeyframe {
        glm::vec3 scale;
        double time;
    };

    struct NodeAnimation {
        int node;
        std::vector<PositionKeyframe> positions;
        std::vector<RotationKeyframe> rotations;
        std::vector<ScaleKeyframe> scales;
    };

    ModelAnimation(std::string name, double duration);

    ModelAnimation(ModelAnimation&&) noexcept = default;
    ~ModelAnimation() noexcept = default;

    std::vector<NodeAnimation>& nodeAnimations();
    const std::vector<NodeAnimation>& nodeAnimations() const;
    std::string name();


private:
    std::string _name;
    double _duration;
    std::vector<NodeAnimation> _nodeAnimations;
};

} // namespace ghoul::io

#endif // __GHOUL___MODELANIMATION___H__
