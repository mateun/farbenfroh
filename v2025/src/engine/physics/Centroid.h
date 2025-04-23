//
// Created by mgrus on 25.03.2025.
//

#ifndef CENTROID_H
#define CENTROID_H

#include <glm\glm.hpp>

/**
* A centroid represents the original 3 points,
* and the average value.
*/
struct Centroid {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;
    glm::vec3 value;
};

#endif //CENTROID_H
