//
// Created by mgrus on 26.03.2025.
//

#ifndef CAMERACOLLIDER_H
#define CAMERACOLLIDER_H

#include <vector>
#include <glm/glm.hpp>



/**
 * This class gets a list of objects which it may not collide with.
 * E.g. a planet, wals, etc.
 * The component takes care if any collision took place and can be asked by the camera
 * to avoid further movement into the colliding object.
 */
class CameraCollider {

public:
    struct CollisionCandidate {
        glm::vec3 location;
        float radius;
    };

    bool collides(glm::vec3 cameraPosition);

    CameraCollider(std::vector<CollisionCandidate*>);

private:
    std::vector<CollisionCandidate*> collisionCandidates;


};



#endif //CAMERACOLLIDER_H
