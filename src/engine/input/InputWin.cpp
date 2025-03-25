//
// Created by mgrus on 25.03.2025.
//

#include "Input.h"
#include <cinttypes>
#include <Windows.h>
#include "Xinput.h"

static Input* inputSingleton = nullptr;

Input* Input::getInstance() {
  if (!inputSingleton) {
    inputSingleton = new InputWin32();

    }
    return inputSingleton;
}

void InputWin32::updateLastKeyPress(uint64_t* uint) {
  lastKeyPressed_ = *uint;
}

// We support Windows VK_ macros here.
bool InputWin32::wasKeyPressed(int key) const {
  return key == lastKeyPressed_;
}

bool InputWin32::pollController(int index) {
    XINPUT_STATE state;
    auto result = XInputGetState(index, &state);
    if (result == ERROR_SUCCESS) {
        controllerStates[index] = state;
    }
    return result == ERROR_SUCCESS;
}

void InputWin32::postPollController(int index) {
    prevControllerStates[index] = controllerStates[index];
}

float InputWin32::getControllerAxis(ControllerAxis axis, int index) {

    if (axis == ControllerAxis::LSTICK_X) {
        return controllerStates[index].Gamepad.sThumbLX / 32767.0f;
    } if ( axis == ControllerAxis::LSTICK_Y) {
        return controllerStates[index].Gamepad.sThumbLY / 32767.0f;
    } if ( axis == ControllerAxis::RSTICK_X) {
        return controllerStates[index].Gamepad.sThumbRX / 32767.0f;
    } if ( axis == ControllerAxis::RSTICK_Y) {
        return controllerStates[index].Gamepad.sThumbRY / 32767.0f;
    } if ( axis == ControllerAxis::L_TRIGGER) {
        return controllerStates[index].Gamepad.bLeftTrigger / 255.0f;
    } if ( axis == ControllerAxis::R_TRIGGER) {
        return controllerStates[index].Gamepad.bRightTrigger / 255.0f;
    }
    return 0;

}

bool InputWin32::controllerButtonPressed(ControllerButtons button, int controllerIndex) {

    auto xiButton = XINPUT_GAMEPAD_A;

    if (button == ControllerButtons::B_BUTTON) {
        xiButton = XINPUT_GAMEPAD_B;
    }
    else if (button == ControllerButtons::X_BUTTON) {
        xiButton = XINPUT_GAMEPAD_X;
    }
    else if (button == ControllerButtons::Y_BUTTON) {
        xiButton = XINPUT_GAMEPAD_Y;
    }
    else if (button == ControllerButtons::DPAD_DOWN) {
        xiButton = XINPUT_GAMEPAD_DPAD_DOWN;
    }
    else if (button == ControllerButtons::DPAD_UP) {
        xiButton = XINPUT_GAMEPAD_DPAD_UP;
    }
    else if (button == ControllerButtons::DPAD_RIGHT) {
        xiButton = XINPUT_GAMEPAD_DPAD_RIGHT;
    }
    else if (button == ControllerButtons::DPAD_LEFT) {
        xiButton = XINPUT_GAMEPAD_DPAD_LEFT;
    }
    else if (button == ControllerButtons::MENU) {
        xiButton = XINPUT_GAMEPAD_START;
    }
    else if (button == ControllerButtons::VIEW) {
        xiButton = XINPUT_GAMEPAD_BACK;
    }
    else if (button == ControllerButtons::LB) {
        xiButton = XINPUT_GAMEPAD_LEFT_SHOULDER;
    }
    else if (button == ControllerButtons::RB) {
        xiButton = XINPUT_GAMEPAD_RIGHT_SHOULDER;
    }

    bool wasPressed = (prevControllerStates[controllerIndex].Gamepad.wButtons & xiButton) != 0;
    bool isPressed = (controllerStates[controllerIndex].Gamepad.wButtons & xiButton) != 0;

    if (!wasPressed && isPressed) {
        return true;
    }

    return false;


}


void InputWin32::initXInput() {
  controllerStates.clear();
  controllerStates.resize(XUSER_MAX_COUNT);
  prevControllerStates.clear();
  prevControllerStates.resize(XUSER_MAX_COUNT);


}

bool Input::isKeyDown(char key) {
    return GetKeyState(key) & 0x8000;
}

void Input::update() {
  POINT pos;
  GetCursorPos(&pos);
  mouse_x_ = pos.x;
  mouse_y_ = pos.y;
}


int Input::mouse_x() {
  return mouse_x_;
}

int Input::mouse_y() {
  return mouse_y_;
}