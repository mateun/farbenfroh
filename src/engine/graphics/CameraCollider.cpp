//
// Created by mgrus on 26.03.2025.
//

#include "CameraCollider.h"

bool CameraCollider::collides(glm::vec3 cameraPosition) {
    for (auto cc : collisionCandidates) {
        if (glm::distance(cameraPosition, cc->location) < (cc->radius + 0.2)) {
            return true;
        }
    }
    return false;
}

CameraCollider::CameraCollider(std::vector<CollisionCandidate *> collisionCandidates) : collisionCandidates(collisionCandidates) {

}