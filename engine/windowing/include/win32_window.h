//
// Created by mgrus on 23.04.2025.
//

#ifndef WIN32_WINDOW_H
#define WIN32_WINDOW_H

#include <Windows.h>
#include <string>

bool mouse_left_up();
bool mouse_left_down();
int win_mouse_x();
int win_mouse_y();
WPARAM last_key_press();
HWND create_window(int w, int h, bool fullscreen, HINSTANCE instance, const std::string& window_title = "Game");
bool poll_window(HWND);

#endif //WIN32_WINDOW_H
