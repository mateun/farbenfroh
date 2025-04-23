//
// Created by mgrus on 23.04.2025.
//

#include <engine.h>

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int) {

    auto win = create_window(800, 600, false, GetModuleHandle(NULL));
    while (1) {
        poll_window(win);
    }

}
