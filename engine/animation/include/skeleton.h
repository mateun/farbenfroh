//
// Created by mgrus on 25.03.2025.
//

#ifndef SKELETON_H
#define SKELETON_H

#include <vector>
#include <string>

class Joint;

struct Skeleton {
    std::vector<Joint*> joints;
    void resetToBindPose();
    Joint* findJointByName(const std::string &name);
};



#endif //SKELETON_H
