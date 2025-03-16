//
// Created by mgrus on 14.02.2025.
//

#include "CharacterController.h"


CharacterController::CharacterController(SceneNode *characterNode) : _characterNode(characterNode){

}

void CharacterController::setRotationSpeed(float value) {
    this->rotationSpeed = value;
}

void CharacterController::setMovementSpeed(float value) {
    this->movementSpeed = value;
}

/**
* The character controller takes input and translates it to movement of the
* attached character SceneNode
*/
void CharacterController::update() {
    float frameMovementSpeed = movementSpeed * ftSeconds;
    float frameRotationSpeed = rotationSpeed * ftSeconds;
    float dir = 0;
    float hdir = 0;
    float yaw = 0;
    float lookHor = 0;
    float lookVer = 0;


    // TODO: instead of directly using specific bindings, we could let us inject the actual
    // values for yaw, pitch etc. from the consumer.
    // Alternative: use a keybinding map injected from outside.
    if (isKeyDown('E') || isKeyDown(VK_RIGHT) || getControllerAxis(ControllerAxis::RSTICK_X, 0) > 0.25) {
        yaw = -1;
        lookHor = -1;
    }

    if (isKeyDown('Q') || isKeyDown(VK_LEFT) || getControllerAxis(ControllerAxis::RSTICK_X, 0) < -0.25) {
        yaw = 1;
        lookHor = 1;
    }

    if (getControllerAxis(ControllerAxis::RSTICK_Y, 0) > 0.25) {
        lookVer = -1;
    }

    if (getControllerAxis(ControllerAxis::RSTICK_Y, 0) < -0.25) {
        lookVer = 1;
    }


    if (isKeyDown('W') || getControllerAxis(ControllerAxis::LSTICK_Y, 0) > 0.25 ) {
        dir = 1;
    }
    if (isKeyDown('S') || getControllerAxis(ControllerAxis::LSTICK_Y, 0) < -0.25) {
        dir = -1;
    }


    if (isKeyDown('A') || getControllerAxis(ControllerAxis::LSTICK_X, 0) < -0.25) {
        hdir = -1;
    }
    if (isKeyDown('D') || getControllerAxis(ControllerAxis::LSTICK_X, 0) > 0.25) {
        hdir = 1;
    }


    // Apply yaw rotation
    _characterNode->yaw(yaw * frameRotationSpeed);

    glm::vec3 loc = _characterNode->getLocation();

    // TODO
    // Here we need to decide if our movement shall be guided by our orientation, or if we always
    // move globally north, south,east and west.
    // For top down shooters the latter is the better.
    if (movementMode == MovementMode::FORWARD) {
        // Forward
        loc += glm::vec3{frameMovementSpeed * _characterNode->getForwardVector().x, 0, frameMovementSpeed * _characterNode->getForwardVector().z} * dir;

        // Strafing
        loc += glm::vec3{frameMovementSpeed * _characterNode->getRightVector().x * 0.33, 0, frameMovementSpeed * _characterNode->getRightVector().z * 0.33} * hdir;
    }

    else if (movementMode == MovementMode::GLOBAL) {
        glm::vec3 globalFwd = (glm::vec3{hdir, 0, -dir});
        loc += glm::vec3{frameMovementSpeed * globalFwd.x, 0, frameMovementSpeed * globalFwd.z};

        // Orientation
        glm::vec3 lookDir = glm::normalize(glm::vec3{lookHor, 0, -lookVer});
        if (length(lookDir) > 0) {
            //player->forward = glm::normalize(glm::vec3(shootDir.x, 0, -shootDir.y));
            float yawAngle = glm::atan(lookDir.x, lookDir.z);

            glm::quat currentOrientation = _characterNode->getWorldOrientation();
            auto targetOrientation = angleAxis(yawAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            // Calculate an interpolation factor (t).
            // This should be between 0 and 1 and can be based on your frame's delta time and a smoothing speed.
            float smoothingSpeed = 20; // adjust as needed
            float deltaTime = ftSeconds;
            float t = glm::clamp(smoothingSpeed * deltaTime, 0.0f, 1.0f);

            // Interpolate using slerp.
            glm::quat smoothOrientation = glm::slerp(currentOrientation, targetOrientation, t);

            // Set the new orientation.
            _characterNode->setOrientation(smoothOrientation);

        }

    }



    _characterNode->setLocation(loc);

}