//
// Created by mgrus on 19.03.2025.
//

#include "NodeComponent.h"

glm::vec3 NodeTransform::localPosition() {
    return _position;
}


NodeTransform NodeComponent::getTransform() {
    NodeTransform transform;
    transform._position  = node->getLocation();
    transform._orientation = node->getOrientation();
    transform._scale = node->getScale();
    // TODO the rest ...
    return transform;
}

void NodeComponent::setPosition(glm::vec3 position) {
    node->setLocation(position);
}

void NodeComponent::disable() {
    node->disable();
}

void ScriptComponent::invoke() {
    update();
}
