//
// Created by mgrus on 25.08.2024.
//

#ifndef SIMPLE_KING_DUNGEON_SHOOT_H
#define SIMPLE_KING_DUNGEON_SHOOT_H

#include "../src/engine/game/default_game.h"
#include "../src/engine/physics/Particle.h"
#include "../src/engine/editor/editor.h"

namespace ds {

    struct Vertex {
        glm::vec3 position;

    };

    struct SkeletalMesh {
        Skeleton* skeleton = nullptr;
        std::vector<Vertex*> vertices;
    };

    struct Joint {
        glm::vec3 position;
        glm::vec3 orientation;
        Joint* parent = nullptr;
        std::vector<Joint*> children;


        // Up to 4 vertices can be influenced by this joint
        std::vector<Vertex*> vertices;

        // Store the weight per vertex.
        // Linked via the index into the vertices vector.
        // So the value at index0 refers to vertex at index0.
        std::vector<float> weights;
    };

    struct Skeleton {
        std::string name;
        Joint* rootJoint = nullptr;
    };
}


// The primary object is a plane.
// Planes can be extruded by their edges.
struct PrimObject {
    std::string name;
    glm::vec3 pivotPosition;
    glm::vec3 rotation;
    glm::vec3 scale = {1, 1, 1};
    bool hasCurrentSelectedVertex = false;
    glm::vec2 selectedVertexScreenCoordinates;


};

/**
 * PhysicsObject represents a game object which
 * position and movement follow the laws of physics.
 * Such an object position is normally not set directly,
 * but is the result of forces acting upon this object.
 */
class PhysicsObject {

public:
    PhysicsObject(glm::vec3 initialPosition, float mass, Mesh* mesh) {
        this->particle = new cyclone::Particle(initialPosition, {0, 0, 0}, mass);
        this->mesh = mesh;
    }

    void update(float duration) {
        if (simulate) {
            this->particle->integrate(duration);
        }
    }

    void applyForce(glm::vec3 force) {
        this->particle->addForce(force);
    }

    glm::vec3 getPosition() {
        return particle->getPosition();
    }

    Mesh* getMesh() {
        return this->mesh;
    }

    void stopSimulation() {
        this->simulate = false;
    }

private:
    cyclone::Particle* particle = nullptr;
    Mesh* mesh = nullptr;
    bool simulate = true;
};

class DungeonGame : public DefaultGame {

public:
    void update() override;
    void init() override;
    void render() override;
    bool shouldStillRun() override;
    bool shouldAutoImportAssets() override;
    std::string getAssetFolder() override;


private:
    Mesh* _houseMesh = nullptr;

    std::vector<PrimObject*> _primObjects;
    std::vector<Skeleton*> _skeletons;

    void renderPrimObjects();
    void renderPhysicsObjects();

    std::vector<PhysicsObject*> _physicsObjects;

    Editor* editor = nullptr;

    // Show the ingame editor, toggled by F10 key.
    bool showEditor = false;
};


#endif //SIMPLE_KING_DUNGEON_SHOOT_H
