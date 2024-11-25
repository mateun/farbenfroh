//
// Created by mgrus on 19.11.2024.
//

#include <glm/glm.hpp>
#include "game_model.h"
#include "../../graphics.h"

glm::vec3 GameObject::collideWithRay(Ray ray) {
    if (!mesh) {
        // We return a length zero vector in case we don't have a mesh.
        return {0, 0, 0};
    }

    // Transform to world space:
    glm::mat4 mattrans = translate(glm::mat4(1), location);
    glm::mat4 matscale = glm::scale(glm::mat4(1),collisionMeshScale);
    glm::mat4 matworld = mattrans * matscale;
    //matworld =  mattrans * matrotX * matrotY * matrotZ * matscale ;

    // Otherwise we check the collision of each triangle (face) within this mesh.
    int count = 0;
    glm::vec3 triangle[3];
    for (auto index : mesh->indices) {
        triangle[count] = mesh->positions[index];
        // Collect first triangle
        if (count == 2) {
            count = 0;

            // Process the actual triangle
            {

                glm::vec3 worldA = matworld * glm::vec4(triangle[0], 1);
                glm::vec3 worldB = matworld * glm::vec4(triangle[1], 1);
                glm::vec3 worldC = matworld * glm::vec4(triangle[2], 1);

                auto edge1 = worldB - worldA;
                auto edge2 = worldC - worldA;
                auto normal = glm::normalize(glm::cross(edge1, edge2));

                // Is this face normal pointing away from our incoming direction, i.e. opposite,
                // then we make further checks.
                if (glm::dot(ray.direction, normal) < 0 ) {
                    // With this normal we can create a plane to check the ray against:
                    glm::vec3 intersectionPoint;
                    if (rayIntersectsPlane(ray, normal, worldA, &intersectionPoint)) {
                        if (pointInTri(intersectionPoint, worldA, worldB, worldC)) {
                            return normal;
                        }

                    }
                }


            }

        } else {
            count++;
        }


    }

    return glm::vec3();
}

GameComponent::GameComponent(GameObject *gameObject) : gameObject(gameObject) {

}
