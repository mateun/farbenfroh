//
// Created by mgrus on 25.03.2025.
//

#ifndef CAMERAMOVER_H
#define CAMERAMOVER_H

#include <engine/input/UpdateSwitcher.h>
#include <glm/glm.hpp>

class Camera;
class CameraCollider;


/**
 * CameraMove can process inputs to move a camera around.
 * Mainly useable for debug cameras with classical WASD movement scheme.
 */
class CameraMover : public Updatable {

public:
    CameraMover(Camera* cam, CameraCollider* cameraCollider = nullptr);
    void update() override;
    void setMovementSpeed(float val);
    void setFixedPlaneForwardMovement(bool b);

    // E.g. to avoid pitch clipping and being lost
    void reset();

private:
    Camera* _cam;
    CameraCollider* cameraCollider;

    float movementSpeed = 5;
    bool fixedPlaneFwdMovement = false;
    bool clampPitch = false;
    glm::vec3 originalLocation;
    glm::vec3 originalTarget;
};


#endif //CAMERAMOVER_H
