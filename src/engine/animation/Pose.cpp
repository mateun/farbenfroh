//
// Created by mgrus on 18.02.2025.
//

#include "Pose.h"
#include "Joint.h"

Joint * Pose::findJointByName(const std::string &name) {
    // TODO make more efficient, maybe map etc.
    for (auto j : joints) {
        if (j->name == name) {
            return j;
        }
    }

    return nullptr;
}

Joint * Pose::findJointByName(const std::string &name, const std::vector<Joint *> &joints) {
    for (auto j : joints) {
        if (j->name == name) {
            return j;
        }
    }
    return nullptr;
}


