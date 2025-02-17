//
// Created by mgrus on 17.02.2025.
//

#ifndef ANIMATION_H
#define ANIMATION_H

#include <string>
#include <vector>
#include <map>
#include <glm\glm.hpp>
#include "glm/detail/type_quat.hpp"

struct AnimationSample {
    std::string jointName;
    float time;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 scale;
};

struct Animation {
    std::string name;
    float duration;
    int frames;
    std::map<std::string,std::vector<AnimationSample*>*> samplesPerJoint;
    double ticksPerSecond;
};

#endif //ANIMATION_H
