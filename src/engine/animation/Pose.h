//
// Created by mgrus on 18.02.2025.
//

#ifndef POSE_H
#define POSE_H

#include <glm/glm.hpp>
#include "../../graphics.h"


class Pose {
  public:
    std::vector<Joint*> joints;

    Joint* findJointByName(const std::string & name);
    static Joint* findJointByName(const std::string & name, const std::vector<Joint*>& joints);
    static glm::mat4 calculateWorldTransform(Joint* j, glm::mat4 currentTransform);
};



#endif //POSE_H
