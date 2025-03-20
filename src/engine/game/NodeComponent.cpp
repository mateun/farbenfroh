//
// Created by mgrus on 19.03.2025.
//

#include "NodeComponent.h"

NodeTransform::NodeTransform() {
}

NodeTransform::NodeTransform(std::shared_ptr<NodeTransform> parent) : _parent(parent) {
}

glm::vec3 NodeTransform::localPosition() const {
    return _localPosition;
}

glm::vec3 NodeTransform::localScale() const {
    return _localScale;
}

glm::quat NodeTransform::localOrientation() const {
    return _localOrientation;
}

glm::vec3 NodeTransform::forward() {
    return _forward;
}

glm::vec3 NodeTransform::position() const {
    if (_parent) {
        return _localPosition + parent()->localPosition();
    }
    return _localPosition;
}

void NodeTransform::collectParents(std::vector<std::shared_ptr<NodeTransform>>& parents) const {
    if (_parent) {
        parents.push_back(_parent);
        _parent->collectParents(parents);
    }

}

glm::quat NodeTransform::orientation() const {
    std::vector<std::shared_ptr<NodeTransform>> flatParents;
    collectParents(flatParents);
    //std::ranges::reverse(flatParents);

    glm::quat aggOrientation = _localOrientation;
    for (auto p : flatParents) {
        aggOrientation = p->_localOrientation * aggOrientation;
    }
    return aggOrientation;
}

std::shared_ptr<NodeTransform> NodeTransform::parent() const {
    return _parent;
}

void NodeComponent::disable() {
    enabled = false;
}

void NodeComponent::enable() {
    enabled = true;
}

SceneNode * NodeComponent::getNode() {
    return node;
}

void NodeComponent::setNode(SceneNode *node) {
    this->node = node;
}

void ScriptComponent::invoke() {
    update();
}
