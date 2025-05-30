//
// Created by mgrus on 29.03.2025.
//

#include "MessageTransformer.h"

#include <windowsx.h>
#include <engine/graphics/Application.h>
#include <engine/graphics/RawWin32Message.h>
#include <engine/graphics/Widget.h>
#include <engine/graphics/ui/UIMessage.h>


UIMessage MessageTransformer::transform(RawWin32Message message) {
    UIMessage result;
    result.num = message.num;
    switch (message.message) {
        case WM_MOUSEMOVE: {
            result.type = MessageType::MouseMove;
            result.mouseMoveMessage.x = GET_X_LPARAM(message.lParam);
            result.mouseMoveMessage.y = getApplication()->scaled_height()- GET_Y_LPARAM(message.lParam);
            break;
        }

        case WM_KEYUP:
            result.type = MessageType::KeyDown;
            result.keyboardMessage.key = message.wParam;
            break;

        case WM_LBUTTONDOWN: {
            result.type = MessageType::MouseDown;
            break;
        }
        case WM_LBUTTONUP: {
            result.type = MessageType::MouseUp;
            break;
        }
        case WM_CHAR:
        {
            char ch = (char) message.wParam;
            result.type = MessageType::Character;
            result.character = ch;
            break;
        }

    }

    return result;

}
