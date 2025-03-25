//
// Created by mgrus on 24.03.2025.
//

#ifndef POSITIONPROVIDER_H
#define POSITIONPROVIDER_H

#include <glm/glm.hpp>

class PositionProvider {
public:
    virtual glm::vec2 getPosition() = 0;
    virtual glm::vec3 getPosition3D()  = 0;
};



#endif //POSITIONPROVIDER_H
