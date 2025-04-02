//
// Created by mgrus on 29.03.2025.
//

#include "MessageDispatcher.h"

#include <engine/graphics/Application.h>
#include <engine/graphics/RawWin32Message.h>

#include <engine/graphics/ui/MessageTransformer.h>

#include "FloatingWindow.h"
#include "FocusManager.h"
#include "MessageHandleResult.h"

SimpleMessageDispatcher::SimpleMessageDispatcher(std::shared_ptr<Widget> topLevelWidget): top_level_widget_(
    std::move(topLevelWidget)) {
}

void SimpleMessageDispatcher::onFrameMessages(const std::vector<RawWin32Message> &msgs) {
    auto floatingWindows = getApplication()->getFloatingWindows();
    for (auto& msg : msgs) {
        auto transformedMessage = MessageTransformer::transform(msg);
        top_level_widget_->onMessage(transformedMessage);
        for (auto fw: floatingWindows) {
            fw->onMessage(transformedMessage);
        }
    }
}

FocusBasedMessageDispatcher::FocusBasedMessageDispatcher(FocusManager &focusManager): focus_manager_(focusManager) {
}

void FocusBasedMessageDispatcher::onFrameMessages(const std::vector<RawWin32Message> &msgs) {
    // TODO ask FocusManager for the currently focused widget.
    // Then transform relevant messages into UIMessage objects and pass them on.
    auto focused_widget = focus_manager_.getFocusedWidget();
    if (!focused_widget) {
        return;
    }

    for (const auto &msg : msgs) {
        auto transformed = MessageTransformer::transform(msg);
        focused_widget->onMessage(transformed);
    }
}

