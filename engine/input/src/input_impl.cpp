//
// Created by mgrus on 29.05.2025.
//
#include "../include/input.h"
#include <engine.h>

bool mouseLeftUp() {
    return mouse_left_up();
}

bool mouseLeftDown() {
    return mouse_left_down();
}

int mouseX() {
    return win_mouse_x();
}

int mouseY() {
    return win_mouse_y();
}

// We support Windows VK_ macros here.
bool keyPressed(int key) {
    //    return GetAsyncKeyState(key) & 0x01;
    return key == last_key_press();
}

bool isKeyDown(int key) {
    return GetKeyState(key) & 0x8000;
}