//
// Created by mgrus on 25.03.2025.
//

#ifndef RAY_H
#define RAY_H

#include <glm\glm.hpp>

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
    float maxLength = 100;
};

#endif //RAY_H
