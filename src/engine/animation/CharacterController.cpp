//
// Created by mgrus on 14.02.2025.
//

#include "CharacterController.h"


CharacterController::CharacterController(SceneNode *characterNode) : _characterNode(characterNode){

}

/**
* The character controller takes input and translates it to movement of the
* attached character SceneNode
*/
void CharacterController::update() {
    float frameMovementSpeed = baseSpeed * ftSeconds;
    float frameRotationSpeed = baseSpeed * ftSeconds * 30;
    float dir = 0;
    float hdir = 0;
    float yaw = 0;


    // TODO: instead of directly using specific bindings, we could let us inject the actual
    // values for yaw, pitch etc. from the consumer.
    // Alternative: use a keybinding map injected from outside.
    if (isKeyDown('E') || isKeyDown(VK_RIGHT) || getControllerAxis(ControllerAxis::RSTICK_X, 0) > 0.25) {
        yaw = -1;
    }

    if (isKeyDown('Q') || isKeyDown(VK_LEFT) || getControllerAxis(ControllerAxis::RSTICK_X, 0) < -0.25) {
        yaw = 1;
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

    // Forward
    loc += glm::vec3{frameMovementSpeed * _characterNode->getForwardVector().x, 0, frameMovementSpeed * _characterNode->getForwardVector().z} * dir;


    // Strafing
    loc += glm::vec3{frameMovementSpeed * _characterNode->getRightVector().x * 0.33, 0, frameMovementSpeed * _characterNode->getRightVector().z * 0.33} * hdir;

    _characterNode->setLocation(loc);

}