//
// Created by mgrus on 02.11.2024.
//

#ifndef SIMPLE_KING_INPUT_H
#define SIMPLE_KING_INPUT_H

enum class ControllerAxis {
    LSTICK_X,
    LSTICK_Y,
    RSTICK_X,
    RSTICK_Y,
    R_TRIGGER,
    L_TRIGGER

};

enum class ControllerButtons {
    A_BUTTON,
    B_BUTTON,
    X_BUTTON,
    Y_BUTTON,
    RB,
    LB,
    DPAD_DOWN,
    DPAD_UP,
    DPAD_RIGHT,
    DPAD_LEFT,
    MENU,
    VIEW
};

// Must be called every frame for each controller index:
bool pollController(int index);

// Has to be called at the end of all single press controller queries to
// avoid longer running detections:
void postPollController(int index);

float getControllerAxis(ControllerAxis axis, int index);
bool controllerButtonPressed(ControllerButtons button, int controllerIndex);

#endif //SIMPLE_KING_INPUT_H
