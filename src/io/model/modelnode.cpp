/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2024                                                               *
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

#include <ghoul/io/model/modelnode.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/fmt.h>
#include <glm/gtc/type_ptr.hpp>

namespace ghoul::io {

ModelNode::ModelNode(glm::mat4x4 transform, std::vector<io::ModelMesh> meshes)
    : _meshes(std::move(meshes))
{
   // glm is column major, array is column major too
    _transform[0] = transform[0][0];
    _transform[1] = transform[0][1];
    _transform[2] = transform[0][2];
    _transform[3] = transform[0][3];

    _transform[4] = transform[1][0];
    _transform[5] = transform[1][1];
    _transform[6] = transform[1][2];
    _transform[7] = transform[1][3];

    _transform[8] = transform[2][0];
    _transform[9] = transform[2][1];
    _transform[10] = transform[2][2];
    _transform[11] = transform[2][3];

    _transform[12] = transform[3][0];
    _transform[13] = transform[3][1];
    _transform[14] = transform[3][2];
    _transform[15] = transform[3][3];
}

void ModelNode::setParent(int parent) {
    _parent = parent;
}

void ModelNode::setChildren(std::vector<int> children) {
    _children = std::move(children);
}

void ModelNode::addChild(int child) {
    _children.push_back(child);
}

void ModelNode::setAnimation(const glm::mat4x4& animation) {
    // glm is column major, array is column major too
    _animationTransform[0] = animation[0][0];
    _animationTransform[1] = animation[0][1];
    _animationTransform[2] = animation[0][2];
    _animationTransform[3] = animation[0][3];

    _animationTransform[4] = animation[1][0];
    _animationTransform[5] = animation[1][1];
    _animationTransform[6] = animation[1][2];
    _animationTransform[7] = animation[1][3];

    _animationTransform[8] = animation[2][0];
    _animationTransform[9] = animation[2][1];
    _animationTransform[10] = animation[2][2];
    _animationTransform[11] = animation[2][3];

    _animationTransform[12] = animation[3][0];
    _animationTransform[13] = animation[3][1];
    _animationTransform[14] = animation[3][2];
    _animationTransform[15] = animation[3][3];

    _hasAnimation = true;
}

std::vector<io::ModelMesh>& ModelNode::meshes() {
    return _meshes;
}

const std::vector<io::ModelMesh>& ModelNode::meshes() const {
    return _meshes;
}

int ModelNode::parent() const {
    return _parent;
}

std::vector<int>& ModelNode::children() {
    return _children;
}

const std::vector<int>& ModelNode::children() const {
    return _children;
}

glm::mat4x4 ModelNode::transform() const {
    return glm::make_mat4(_transform);
}

glm::mat4x4 ModelNode::animationTransform() const {
    return glm::make_mat4(_animationTransform);
}

bool ModelNode::hasAnimation() const {
    return _hasAnimation;
}

} // namespace ghoul::io
