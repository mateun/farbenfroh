//
// Created by mgrus on 19.03.2025.
//

#include "SceneNode.h"

#include "NodeComponent.h"



SceneNode::SceneNode(const std::string &nodeId, std::shared_ptr<NodeTransform> transform) : id (nodeId), _transform(transform){
}

SceneNode::~SceneNode() {
}

Camera * SceneNode::getCamera() const {
    return camera;
}


void SceneNode::setOrientation(glm::quat orientation) {
    this->orientation = orientation;
}

glm::vec3 SceneNode::getLocation() {
    return _location;
}

glm::vec3 SceneNode::getScale() {
    return _scale;
}


std::string SceneNode::getId() {
    return id;
}

void SceneNode::yaw(float degrees) {

    auto yawQuat =angleAxis(glm::radians(degrees), glm::vec3(0.0f, 1.0f, 0.0f));
    orientation = yawQuat * orientation;
    //orientation = yawQuat;

}

glm::vec3 SceneNode::getRightVector() {
    return right;
}

glm::vec3 SceneNode::getVelocity() {
    return _velocity;
}

bool SceneNode::isActive() {
    return _active;
}

void SceneNode::setLocation(glm::vec3 vec) {
    _location = vec;
    if (_type == SceneNodeType::Camera) {
        camera->updateLocation(_location);
    }
}

void SceneNode::setScale(glm::vec3 scale) {
    _scale = scale;
}

void SceneNode::setRotation(glm::vec3 rotationInEulers, AngleUnit unit) {
    if (unit == AngleUnit::RAD) {
        _rotationInDeg = degrees(rotationInEulers);
    } else {
        _rotationInDeg = rotationInEulers;
    }

    if (_type == SceneNodeType::Camera) {
        glm::qua rotQ = glm::qua(rotationInEulers);
        glm::mat3 rotMat = glm::toMat3(rotQ);
        auto newFwd = (normalize(rotMat * camera->_initialForward));
        camera->updateLookupTarget(camera->location + newFwd);

    }
}





void SceneNode::updateBoneMatrices(std::vector<glm::mat4> boneMatrices) {
    _boneMatrices = boneMatrices;
}

const std::vector<glm::mat4> & SceneNode::boneMatrices() {
    return _boneMatrices;
}

void SceneNode::disable() {
    _active = false;
}

void SceneNode::enable() {
    _active = true;
}

void SceneNode::setParent(const std::shared_ptr<SceneNode>& parentNode) {
    this->parent = parentNode;

}

void SceneNode::addChild(std::shared_ptr<SceneNode> child) {
    child->setParent(shared_from_this());
    children.push_back(std::move(child));
}

MeshDrawData SceneNode::getMeshData() {
    return meshData;
}

void SceneNode::setExtraData(void *data) {
    this->extraData = data;
}

void * SceneNode::getExtraData() {
    return extraData;
}

void SceneNode::update() {
    for (auto& nc : _components) {
        nc->invoke();
    }
}

void SceneNode::addComponent(std::unique_ptr<NodeComponent> component) {
    component->setNode(shared_from_this());
    _components.push_back(std::move(component));

}

glm::quat SceneNode::getOrientation() {
    return orientation;
}

std::shared_ptr<NodeTransform> SceneNode::transform() {
    return _transform;
}


void SceneNode::initAsCameraNode(Camera *camera) {
    this->_type = SceneNodeType::Camera;
    this->camera = camera;
    this->setLocation(camera->location);
}

void SceneNode::initAsLightNode(Light *light) {
    this->_type = SceneNodeType::Light;
    this->light = light;
    this->setLocation(light->location);

}

glm::vec3 SceneNode::getForwardVector() {
    glm::vec3 defaultForward = {0, 0, -1};
    return normalize(orientation * defaultForward);
}


void SceneNode::initAsMeshNode(const MeshDrawData& meshData) {
    this->_type = SceneNodeType::Mesh;
    this->mesh = meshData.mesh;
    this->texture = meshData.texture;
    this->normalMap = meshData.normalMap;
    this->shader = meshData.shader;
    this->uvScale = meshData.uvScale;
    this->uvScale2 = meshData.uvScale2;
    this->uvPan = meshData.uvPan;
    this->skinnedMesh = meshData.skinnedDraw;
    this->foregroundColor = meshData.color;
    this->meshData = meshData;
}

void SceneNode::initAsParticleSystemNode(gru::ParticleSystem *particleSystem) {
    this->_type = SceneNodeType::ParticleSystem;
    this->particleSystem = particleSystem;
}


glm::vec3 SceneNode::getHierarchicalWorldLocation(glm::vec3 localLocation) {
    if (auto p = parent.lock()) {
        return localLocation + p->_location;
    }

    return localLocation;

}

void SceneNode::collectParentNodes(std::vector<std::shared_ptr<SceneNode>>& parents) {
    if (auto p = parent.lock()) {
        parents.push_back(p);
        p->collectParentNodes(parents);
    }

}

glm::quat SceneNode::getWorldOrientation() {
    std::vector<std::shared_ptr<SceneNode>> flatParents;
    collectParentNodes(flatParents);
    //std::ranges::reverse(flatParents);

    glm::quat aggOrientation = orientation;
    for (auto p : flatParents) {
        aggOrientation = p->orientation * aggOrientation;
    }
    return aggOrientation;

}
