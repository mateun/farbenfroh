//
// Created by mgrus on 29.05.2025.
//
#include "../include/input.h"
#include <engine.h>

extern WPARAM lastKeyPress;

// We support Windows VK_ macros here.
bool keyPressed(int key) {
    //    return GetAsyncKeyState(key) & 0x01;
    return key == lastKeyPress;
}

bool isKeyDown(int key) {
    return GetKeyState(key) & 0x8000;
}