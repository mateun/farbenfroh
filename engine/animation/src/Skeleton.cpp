//
// Created by mgrus on 25.03.2025.
//

#include <skeleton.h>
#include <Joint.h>



void Skeleton::resetToBindPose() {
    for (auto j: joints) {
        j->currentPoseLocalTransform = j->bindPoseLocalTransform;
        j->currentPoseLocation.clear();
        j->currentPoseOrientation.clear();
    }
}

Joint* Skeleton::findJointByName(const std::string &name) {
    for (auto j : joints) {
        if (j->name == name) {
            return j;
        }
    }
    return nullptr;
}
