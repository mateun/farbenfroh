//
// Created by mgrus on 29.03.2025.
//

#include "MessageDispatcher.h"
#include <engine/graphics/RawWin32Message.h>

MessageDispatcher::MessageDispatcher(FocusManager &focusManager): focus_manager_(focusManager) {
}

void MessageDispatcher::onFrameMessages(const std::vector<RawWin32Message> &msgs) {
    // TODO ask FocusManager for the currently focused widget.
    // Then transform relevant messages into UIMessage objects and pass them on.

    for (const auto &msg : msgs) {
        // TODO implement
        // ask
    }
}

