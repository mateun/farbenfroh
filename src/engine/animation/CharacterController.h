//
// Created by mgrus on 14.02.2025.
//

#ifndef CHARACTERCONTROLLER_H
#define CHARACTERCONTROLLER_H

#include <graphics.h>

enum class MovementMode {
    FORWARD,
    GLOBAL,
};

class CharacterController : public Updatable {

public:
    CharacterController(SceneNode* characterNode);

    void setRotationSpeed(float value);
    void setMovementSpeed(float value);

    void update() override;

private:
    SceneNode * _characterNode = nullptr;
    float baseSpeed = 10;
    MovementMode movementMode = MovementMode::GLOBAL;
    float movementSpeed = 15;
    float rotationSpeed = 45;
};



#endif //CHARACTERCONTROLLER_H
