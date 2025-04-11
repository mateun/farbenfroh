//
// Created by mgrus on 29.03.2025.
//

#include "MessageDispatcher.h"

#include <iostream>
#include <engine/graphics/Application.h>
#include <engine/graphics/RawWin32Message.h>

#include <engine/graphics/ui/MessageTransformer.h>
#include <engine/graphics/ui/UIMessage.h>

#include "FloatingWindow.h"
#include "FocusManager.h"
#include "MessageHandleResult.h"


void SimpleMessageDispatcher::onFrameMessages(const std::vector<RawWin32Message> &msgs) {
    auto floatingWindows = getApplication()->getFloatingWindows();
    for (auto& msg : msgs) {
        auto transformedMessage = MessageTransformer::transform(msg);
        transformedMessage.sender = "SimpleMessageDispatcher";

        // Send any message to the centralSubMenuManager.
        getApplication()->getCentralSubMenuManager()->onMessage(transformedMessage);

        // Send messages to floating windows, as we assume they are highest in z-order.
        // Stop forwarding the message after the first window handled it.
        MessageHandleResult handleResult = {};
        for (const auto& fw: floatingWindows) {
            handleResult = fw->onMessage(transformedMessage);
            if (handleResult.wasHandled) break;
        }
        // Only forward to static top level widget if it was not handled already.
        if (handleResult.wasHandled) continue;

        if (getApplication()->getMenuBar()) {
            getApplication()->getMenuBar()->onMessage(transformedMessage);
        }

        if (getApplication()->getTopLevelWidget()) {
            getApplication()->getTopLevelWidget()->onMessage(transformedMessage);
        }

    }
}

FocusBasedMessageDispatcher::FocusBasedMessageDispatcher(FocusManager &focusManager): focus_manager_(focusManager) {
}

void FocusBasedMessageDispatcher::onFrameMessages(const std::vector<RawWin32Message> &msgs) {

    // Then transform relevant messages into UIMessage objects and pass them on.
    auto focused_widget = focus_manager_.getFocusedWidget();
    if (!focused_widget) {
        return;
    }

    for (const auto &msg : msgs) {
        msg_number_++;
        auto transformed = MessageTransformer::transform(msg);
        transformed.sender = "focus-based-message-dispatcher";

        std::cout << "msg num: " << std::to_string(msg_number_) << " sent to widget: " << focused_widget->getId() << std::endl;
        focused_widget->onMessage(transformed);
    }
}

