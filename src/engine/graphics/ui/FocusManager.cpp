//
// Created by mgrus on 29.03.2025.
//

#include "FocusManager.h"


#include <windowsx.h>
#include <engine/graphics/Application.h>
#include <engine/graphics/RawWin32Message.h>

FocusManager::FocusManager() {
    printf("in ctr\n");
}

void FocusManager::update() {
    // Find the first matching widget
}

/**
 * We focus on mouse click messages to decide on focus.
 * @param msgs The incoming raw Windows messages from the last frame.
 */
 void FocusManager::onFrameMessages(const std::vector<RawWin32Message>& msgs) {
     for (auto m : msgs) {
         switch (m.message) {
             case WM_MOUSEMOVE: {
                 mouse_x = GET_X_LPARAM(m.lParam);
                 mouse_y = getApplication()->scaled_height() - GET_Y_LPARAM(m.lParam);

             }
         }
     }
}

