//
// Created by mgrus on 05.03.2025.
//

#include "VectorUtils.h"

glm::vec3 VectorUtils::findCenterForWorldPositions(std::vector<glm::vec3> worldCorners) {
    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : worldCorners)
    {
        center += glm::vec3(v);
    }
    center /= worldCorners.size();
    return center;
}

float VectorUtils::findMin(const std::vector<glm::vec3> &positions, const std::string &coord) {
    float min = std::numeric_limits<float>::max();
    for (auto p : positions) {
        if (coord == "x") {
            if (p.x < min) {
                min = p.x;
            }
        } else if (coord == "y") {
            if (p.y < min) {
                min = p.y;
            }
        }
        else if (coord == "z") {
            if (p.z < min) {
                min = p.z;
            }
        }

    }

    return min;
}

float VectorUtils::findMax(const std::vector<glm::vec3> &positions, const std::string &coord) {
    float m = std::numeric_limits<float>::lowest();
    for (auto p : positions) {
        if (coord == "x") {
            if (p.x > m) {
                m = p.x;
            }
        } else if (coord == "y") {
            if (p.y > m) {
                m = p.y;
            }
        }
        else if (coord == "z") {
            if (p.z > m) {
                m = p.z;
            }
        }
    }

    return m;
}
