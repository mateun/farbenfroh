//
// Created by mgrus on 23.04.2025.
//

#ifndef WIN32_WINDOW_H
#define WIN32_WINDOW_H

#include <Windows.h>
#include <symbol_exports.h>
#include <string>

extern "C" ENGINE_API int win_mouse_x();
extern "C" ENGINE_API int win_mouse_y();
extern "C" ENGINE_API WPARAM last_key_press();
//HWND create_window(int w, int h, bool fullscreen, HINSTANCE instance, const std::string& window_title = "Game");
extern "C" ENGINE_API HWND create_window(int w, int h, bool fullscreen, HINSTANCE instance, const std::string& window_title = "Game");
extern "C" ENGINE_API bool poll_window(HWND);

#endif //WIN32_WINDOW_H
