//
// Created by mgrus on 08.04.2025.
//

#ifndef UIMESSAGE_H
#define UIMESSAGE_H

#include <memory>
#include <string>

class Widget;

enum class MessageType {
    KeyUp,
    KeyDown,
    MouseMove,
    MouseUp,
    MouseDown,
    WidgetGainedFocus,
    WidgetLostFocus,
    Character,
};

struct MouseMoveMessage {
    int x;
    int y;
};



struct FocusMessage {
    std::shared_ptr<Widget> widget;
};

struct KeyboardMessage {
    uint32_t key;
};

struct UIMessage {
    MessageType type;
    MouseMoveMessage mouseMoveMessage;
    FocusMessage focusMessage;
    KeyboardMessage keyboardMessage;
    uint64_t num = 0;
    std::string sender;
    char character = '\0';

};


#endif //UIMESSAGE_H
