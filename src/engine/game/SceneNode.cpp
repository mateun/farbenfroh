//
// Created by mgrus on 19.03.2025.
//

#include "SceneNode.h"

#include "NodeComponent.h"

SceneNode::SceneNode(const std::string &nodeId) : id(nodeId){
}

SceneNode::~SceneNode() {
}

Camera * SceneNode::getCamera() {
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
        auto camera = getCamera();
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

void SceneNode::setParent(SceneNode *parent) {
    this->parent = parent;
    parent->children.push_back(this);
}

void SceneNode::addChild(SceneNode* child) {
    child->setParent(this);
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

void SceneNode::udpate() {
    for (auto nc : components) {
        nc->invoke();
    }
}

void SceneNode::addComponent(NodeComponent *component) {
    components.push_back(component);
    component->setNode(this);
}

glm::quat SceneNode::getOrientation() {
    return orientation;
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
    if (parent) {
        return localLocation + parent->_location;
    }

    return localLocation;


}

void SceneNode::collectParentNodes(std::vector<SceneNode*>& parents) {
    if (parent) {
        parents.push_back(parent);
        parent->collectParentNodes(parents);
    }

}

glm::quat SceneNode::getWorldOrientation() {
    std::vector<SceneNode*> flatParents;
    collectParentNodes(flatParents);
    //std::ranges::reverse(flatParents);

    glm::quat aggOrientation = orientation;
    for (auto p : flatParents) {
        aggOrientation = p->orientation * aggOrientation;
    }
    return aggOrientation;

}
