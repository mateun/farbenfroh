//
// Created by mgrus on 23.04.2025.
//

#ifndef WIN32_WINDOW_H
#define WIN32_WINDOW_H

#include <Windows.h>
#include <symbol_exports.h>
#include <string>
extern ENGINE_API WPARAM lastKeyPress;
extern "C" ENGINE_API HWND create_window(int w, int h, bool fullscreen, HINSTANCE instance, const std::string& window_title = "Game");
extern "C" ENGINE_API bool poll_window(HWND);

#endif //WIN32_WINDOW_H
