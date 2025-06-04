//
// Created by mgrus on 29.05.2025.
//

#ifndef INPUT_H
#define INPUT_H

// The current mouse coordinates
int mouseX();
int mouseY();

// For single presses, when typing in a textfield for example.
// We support Windows VK_ macros here.
bool keyPressed(int key);

// For constant checking of e.g. movement keys
bool isKeyDown(int key);

#endif //INPUT_H
