//
// Created by mgrus on 29.05.2025.
//
#include "../include/input.h"
#include <engine.h>


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