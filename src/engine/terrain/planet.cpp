//
// Created by mgrus on 19.10.2024.
//

#include "planet.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>

namespace fireball {


    Sphere::Sphere(int radius, glm::vec3 location) : radius(radius), worldLocation(location) {
        terrainTop = new Terrain(radius * 2, radius * 2, nullptr, radius);

        auto matRight = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0, 0, -1));
        terrainRight = new Terrain(radius * 2, radius * 2, &matRight, radius);

        auto matLeft = glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0, 0, -1));
        terrainLeft = new Terrain(radius * 2, radius * 2, &matLeft, radius);

        auto matBottom = glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(1, 0, 0));
        terrainBottom = new Terrain(radius * 2, radius * 2, &matBottom, radius);

        auto matFront = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1, 0, 0));
        terrainFront = new Terrain(radius * 2, radius * 2, &matFront, radius);

        auto matBack = glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        terrainBack = new Terrain(radius * 2, radius * 2, &matBack, radius);

    }

    void Sphere::render() {
        terrainTop->render();
        terrainRight->render();
        terrainLeft->render();
        terrainBottom->render();
        terrainFront->render();
        terrainBack->render();

    }

    float Sphere::getRadius() {
        return radius;
    }

    glm::vec3 Sphere::getWorldLocation() {
        return worldLocation;
    }

    void Sphere::setWorldLocation(glm::vec3 loc) {
        worldLocation = loc;
    }

}