//
// Created by mgrus on 19.03.2025.
//

#include "NodeComponent.h"

glm::vec3 NodeTransform::localPosition() {
    return _position;
}

NodeTransform NodeComponent::getTransform() {
    return {node->getLocation(), node->getOrientation(), node->getScale()};
}

void NodeComponent::setPosition(glm::vec3 position) {
    node->setLocation(position);
}

void ScriptComponent::invoke() {
    update();
}
