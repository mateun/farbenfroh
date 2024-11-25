//
// Created by mgrus on 29.05.2024.
//

#include "FirstPersonController.h"

namespace mtking {
    FirstPersonController::FirstPersonController(Character *character, Camera* camera) {
        this->_character = character;
        this->_camera = camera;
        _character->position = _camera->location - glm::vec3(0, 1.8, 0);
    }

    // Check inputs and then apply movement to the character
    void FirstPersonController::updateMovement() {

        glm::vec3 fwd = {0, 0, -1};

        // Turning logic
        glm::vec4 tankFwd = {fwd, 1};
        static float yaw = 0;
        yaw -= mouse_rel_x * 200 * ftSeconds;
        auto matYaw= glm::rotate(glm::mat4(1), glm::radians(yaw), {0, 1, 0});
        tankFwd = glm::normalize(matYaw * tankFwd);
        fwd = glm::vec3(tankFwd.x, tankFwd.y, tankFwd.z);
        this->_camera->updateLookupTarget(_camera->location + fwd);
        mouse_rel_x = 0;
        mouse_rel_y = 0;

        if (isKeyDown('W')) {
            this->_character->position += fwd * 20.0f * ftSeconds;
            this->_camera->updateLocation({this->_character->position.x, 1.8, _character->position.z});

        }
        if (isKeyDown('S')) {
            this->_character->position -= fwd * 20.0f * ftSeconds;
            this->_camera->updateLocation({this->_character->position.x, 1.8, _character->position.z});

        }

        // Strafing
        glm::vec3 right = glm::normalize(glm::cross(fwd, {0, 1, 0}));
        if (isKeyDown('D')) {
            this->_character->position += right * 20.0f * ftSeconds;
            this->_camera->updateLocation({this->_character->position.x, 1.8, _character->position.z});
        }

        if (isKeyDown('A')) {
            this->_character->position -= right * 20.0f * ftSeconds;
            this->_camera->updateLocation({this->_character->position.x, 1.8, _character->position.z});
        }



    }
} // mtking