//
// Created by mgrus on 25.03.2025.
//

#ifndef INPUTWIN32_H
#define INPUTWIN32_H

#include <cinttypes>
#include <vector>
#include <engine/input/Input.h>
#include <Xinput.h>

class InputWin32 : public Input {

public:
    void init() override;

    void initXInput();
    bool wasKeyPressed(int key) const override;
    void updateLastKeyPress(uint64_t *uint) override;
    bool pollController(int index) override;
    float getControllerAxis(ControllerAxis axis, int index) override;
    bool controllerButtonPressed(ControllerButtons button, int controllerIndex) override;


protected:
    // Controller (GamePad) states.
    // We need the previous state to enable single button press detection.
    std::vector<XINPUT_STATE> controllerStates;
    std::vector<XINPUT_STATE> prevControllerStates;
};


#endif //INPUTWIN32_H
