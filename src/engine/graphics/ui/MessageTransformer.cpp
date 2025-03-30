//
// Created by mgrus on 29.03.2025.
//

#include "MessageTransformer.h"

#include <windowsx.h>
#include <engine/graphics/RawWin32Message.h>
#include <engine/graphics/Widget.h>


UIMessage MessageTransformer::transform(RawWin32Message message) {
    UIMessage result;
    switch (message.message) {
        case WM_MOUSEMOVE: {
            result.type = MessageType::MouseMove;
            result.mouseMoveMessage.x = GET_X_LPARAM(message.lParam);
            result.mouseMoveMessage.y = GET_Y_LPARAM(message.lParam);
            break;
        }

        case WM_LBUTTONDOWN: {
            result.type = MessageType::MouseDown;
            break;
        }
        case WM_LBUTTONUP: {
            result.type = MessageType::MouseUp;
            break;
        }
    }

    return result;

}
