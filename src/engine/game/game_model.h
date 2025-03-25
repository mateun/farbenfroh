//
// Created by mgrus on 16.09.2024.
//

#ifndef SIMPLE_KING_GAME_MODEL_H
#define SIMPLE_KING_GAME_MODEL_H
#include <vector>
#include <string>
#include "../src/engine/math/math3d.h"
#include <engine/graphics/Mesh.h>


class GameComponent;

struct GameObject {
    std::string name;
    std::vector<GameObject*> children;
    std::vector<GameComponent*> components;
    Mesh* mesh = nullptr;
    float timeAlive = 0;

    glm::vec3 location;
    glm::vec3 rotation;
    glm::quat orientation;
    glm::vec3 forward;
    glm::vec3 scale;
    glm::vec3 collisionMeshScale;
    AABB aabb;

    glm::vec3 collideWithRay(Ray ray);

    AABB getUpdatedAABBByLocation() {
        AABB updatedAABB;
        updatedAABB.minX = location.x + aabb.minX;
        updatedAABB.minY = location.y + aabb.minY;
        updatedAABB.minZ = location.z + aabb.minZ;
        updatedAABB.maxX = location.x + aabb.maxX;
        updatedAABB.maxY = location.y + aabb.maxY;
        updatedAABB.maxZ = location.z + aabb.maxZ;
        return updatedAABB;
    }

    AABB getUpdatedOtherAABBByLocation(AABB other) {
        AABB updatedAABB;
        updatedAABB.minX = location.x + other.minX;
        updatedAABB.minY = location.y + other.minY;
        updatedAABB.minZ = location.z + other.minZ;
        updatedAABB.maxX = location.x + other.maxX;
        updatedAABB.maxY = location.y + other.maxY;
        updatedAABB.maxZ = location.z + other.maxZ;
        return updatedAABB;
    }

};

/**
 * Components allow the extension of GameObjects without changing the actual
 * GameObject class.
 */
class GameComponent {
public:
    GameComponent(GameObject* gameObject);

    virtual void update() = 0;
    virtual void render() = 0;

protected:
    GameObject* gameObject = nullptr;

};



struct Level {
    std::vector<GameObject*> gameObjects;
};


#endif //SIMPLE_KING_GAME_MODEL_H
