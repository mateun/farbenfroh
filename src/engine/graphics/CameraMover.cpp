//
// Created by mgrus on 25.03.2025.
//

#include "CameraMover.h"

#include <engine/game/Timing.h>
#include <engine/input/Input.h>
#include <engine/graphics/CameraCollider.h>

#include "Camera.h"
#include "StatefulRenderer.h"


CameraMover::CameraMover(Camera *cam, CameraCollider* cameraCollider) : cameraCollider(cameraCollider), _cam(cam) {
    originalLocation = cam->location;
    originalTarget = cam->lookAtTarget;
}

// We move the camera with the WASD keys
//and potentially later with a gamepad right stick.
void CameraMover::update() {

    float camspeed = movementSpeed * Timing::lastFrameTimeInSeconds();
    float rotspeed = movementSpeed * Timing::lastFrameTimeInSeconds();
    float dir = 0;
    float hdir = 0;
    float yaw = 0;
    float pitch = 0;

    // This allows moving fwd/bwd in the current plane.
    // So even if you look down pitched, you would still move level.
    float panFwd = 0;

    auto input = Input::getInstance();
    // TODO: instead of directly using specific bindings, we could let us inject the actual
    // values for yaw, pitch etc. from the consumer.
    // Alternative: use a keybinding map injected from outside.
    if (input->isKeyDown('E') || input->getControllerAxis(ControllerAxis::RSTICK_X, 0) > 0.4) {
        yaw = -1;
    }


    if (input->isKeyDown('Q') || input->getControllerAxis(ControllerAxis::RSTICK_X, 0) < -0.4) {
        yaw = 1;
    }

    if (input->isKeyDown(VK_UP) || input->getControllerAxis(ControllerAxis::RSTICK_Y, 0) > 0.4) {
        pitch = -1;
    }

    if (input->isKeyDown(VK_DOWN) || input->getControllerAxis(ControllerAxis::RSTICK_Y, 0) < -0.4) {
        pitch = 1;
    }

    if (input->isKeyDown('W') || input->getControllerAxis(ControllerAxis::R_TRIGGER, 0) > 0.2) {
        dir = 1;
    }
    if (input->isKeyDown('S') || input->getControllerAxis(ControllerAxis::L_TRIGGER, 0) > 0.2) {
        dir = -1;
    }


    if (input->isKeyDown('S') || input->getControllerAxis(ControllerAxis::LSTICK_Y, 0) < -0.3) {
        panFwd = -1;
    }

    if (input->isKeyDown('W') || input->getControllerAxis(ControllerAxis::LSTICK_Y, 0) > 0.3) {
        panFwd = 1;
    }

    if (input->isKeyDown('A') || input->getControllerAxis(ControllerAxis::LSTICK_X, 0) < -0.4) {
        hdir = -1;
    }
    if (input->isKeyDown('D') || input->getControllerAxis(ControllerAxis::LSTICK_X, 0) > 0.4) {
        hdir = 1;
    }

    auto fwd = _cam->getForward();
    auto right = _cam->getRight();
    auto up = _cam->getUp();

    // Update fwd with yaw
    glm::vec4 fwdAfterYaw = glm::rotate(glm::mat4(1), yaw * rotspeed * 0.1f, up) * glm::vec4(fwd, 0);

    // Update fwd with pitch
    // Clamp angle

    auto angle = pitch * rotspeed * 0.1f;
    auto steepness = glm::dot({0, 1, 0}, fwd);

    if (clampPitch) {
        if (steepness < -0.7 && angle < 0) {
            angle = 0;
        }

        if (steepness > -0.2 && angle > 0) {
            angle = 0;
        }
    }

    glm::vec4 fwdAfterPitch = glm::rotate(glm::mat4(1), angle, right) * fwdAfterYaw;

    glm::vec3 loc = _cam->location;


    // Move along the fwd.
    // Check camera collider if we have one.
    if (cameraCollider) {
        // Collision check for forward direction.

        auto locCandidateFwd =  loc + glm::vec3{camspeed * fwdAfterYaw.x, 0, camspeed * fwdAfterYaw.z} * dir;

        // Move always horizontally straight fwd (i.e. without pitch)
        if (!fixedPlaneFwdMovement) {
            locCandidateFwd = loc +  glm::vec3{camspeed * fwdAfterPitch.x, camspeed * fwdAfterPitch.y, camspeed * fwdAfterPitch.z} * dir;
        }

        right = _cam->getRight();
        auto locCandidate = locCandidateFwd + glm::vec3{camspeed * right.x * 0.33, 0, camspeed * right.z * 0.33} * hdir;

        // Check if this would collide
        if (cameraCollider->collides(locCandidate) ) {
            // Do not update the location, as we would collide!
        } else {
            // Fine to update the location with the candidate, as there is no collision detected.
            loc = locCandidate;
        }
    } else {
        // Forward
        if (fixedPlaneFwdMovement ) {

            // Check for lower and upper bounds
            auto locCandidate = loc + glm::vec3{camspeed * fwdAfterYaw.x, 0, camspeed * fwdAfterYaw.z} * dir;
            if (locCandidate.y < 5 || locCandidate.y > 15) {
                // nothing
            } else {
                loc = locCandidate;
            }

        } else {
            if (panFwd != 0) {
                loc += glm::vec3{camspeed * fwdAfterYaw.x, 0, camspeed * fwdAfterYaw.z} * panFwd;
            } else {
                auto locCandidate = loc + glm::vec3{camspeed * fwdAfterPitch.x, camspeed * fwdAfterPitch.y, camspeed * fwdAfterPitch.z} * dir;
                if (locCandidate.y < .5 || locCandidate.y > 555) {
                    // nothing
                } else {
                    loc = locCandidate;
                }

            }

        }


        // Strafing
        right = _cam->getRight();
        loc += glm::vec3{camspeed * right.x * 0.33, 0, camspeed * right.z * 0.33} * hdir;
    }



    auto lookTarget = loc + glm::normalize(glm::vec3(fwdAfterPitch.x, fwdAfterPitch.y, fwdAfterPitch.z));
    _cam->updateLocation({loc.x, loc.y, loc.z});
    _cam->updateLookupTarget(lookTarget);

}

void CameraMover::setMovementSpeed(float val) {
    movementSpeed = val;
}

void CameraMover::setFixedPlaneForwardMovement(bool b) {
    fixedPlaneFwdMovement = b;
}

void CameraMover::reset() {
    _cam->updateLocation(originalLocation);
    _cam->updateLookupTarget(originalTarget);
}