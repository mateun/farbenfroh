//
// Created by mgrus on 14.02.2025.
//

#ifndef CHARACTERCONTROLLER_H
#define CHARACTERCONTROLLER_H

#include <graphics.h>

class CharacterController : public Updatable {

public:
    CharacterController(SceneNode* characterNode);
    void update() override;

private:
    SceneNode * _characterNode = nullptr;
    float baseSpeed = 10;
};



#endif //CHARACTERCONTROLLER_H
