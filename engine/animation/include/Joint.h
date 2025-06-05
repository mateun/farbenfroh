//
// Created by mgrus on 25.03.2025.
//

#ifndef JOINT_H
#define JOINT_H

#include <string>
#include <map>
#include <vector>
#include <glm/gtc/quaternion.hpp>

struct Joint {

    void setParent(Joint* parent) {
        this->parent = parent;
        parent->children.push_back(this);
    }

    std::string name;
    glm::mat4 inverseBindMatrix;

    // These are the transformations in parent space
    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;

    std::map<std::string, glm::vec3> currentPoseLocation;
    std::map<std::string, glm::quat> currentPoseOrientation;
    glm::vec3 currentPoseScale;

    glm::mat4 bindPoseLocalTransform = glm::mat4(1.0f);
    glm::mat4 bindPoseGlobalTransform = glm::mat4(1.0f);
    glm::mat4 bindPoseFinalTransform = glm::mat4(1.0f);

    glm::mat4 currentPoseLocalTransform= glm::mat4(1);
    glm::mat4 currentPoseGlobalTransform = glm::mat4(1);
    // This is the product of the global and inverseBind matrix.
    // Can be used for skinned meshes to move the skeletal mesh vertex
    // according to this transform.
    glm::mat4 currentPoseFinalTransform = glm::mat4(1);

    // These are the transformations as matrices in model space
    glm::mat4 modelTranslation = glm::mat4(1);
    glm::mat4 modelRotation = glm::mat4(1);
    glm::mat4 modelScale = glm::mat4(1);
    std::vector<Joint*> children;
    Joint* parent = nullptr;


};





#endif //JOINT_H
