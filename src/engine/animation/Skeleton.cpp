//
// Created by mgrus on 25.03.2025.
//

#include <engine/animation\skeleton.h>
#include <engine\animation\Joint.h>


Joint* gru::Skeleton::findJointByName(const std::string &name) {
    for (auto j : joints) {
        if (j->name == name) {
            return j;
        }
    }
    return nullptr;
}
