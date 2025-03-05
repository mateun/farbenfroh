//
// Created by mgrus on 05.03.2025.
//

#ifndef VECTORUTILS_H
#define VECTORUTILS_H

#include "string"
#include <vector>
#include <glm/glm.hpp>


class VectorUtils {

public:
    static float findMin(const std::vector<glm::vec3>& positions, const std::string& coord);
    static float findMax(const std::vector<glm::vec3>& positions, const std::string& coord);
    static glm::vec3 findCenterForWorldPositions(std::vector<glm::vec3> worldCorners);


};



#endif //VECTORUTILS_H
