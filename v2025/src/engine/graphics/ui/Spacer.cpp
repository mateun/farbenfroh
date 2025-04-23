//
// Created by mgrus on 02.04.2025.
//

#include "Spacer.h"

#include "MessageHandleResult.h"

Spacer::Spacer(glm::vec2 size) : preferred_size_(size) {
}

glm::vec2 Spacer::getPreferredSize() {
    return preferred_size_;
}

MessageHandleResult Spacer::onMessage(const UIMessage &message) {
    return Widget::onMessage(message);
}


