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
};



#endif //POSE_H
