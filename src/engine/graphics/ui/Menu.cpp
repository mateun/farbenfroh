//
// Created by mgrus on 31.03.2025.
//

#include "Menu.h"
#include <engine/graphics/ui/MessageHandleResult.h>

#include "LabelWidget.h"

Menu::Menu(const std::string& text): text_(text) {
    auto font = std::make_shared<TrueTypeFont>("../assets/calibri.ttf", 13);
    label_ = std::make_shared<LabelWidget>(text, font);
}

void Menu::draw() {
    label_->setOrigin(origin_ + glm::vec2{2, 2});
    label_->setSize(size_);

    label_->draw();
}

MessageHandleResult Menu::onMessage(const UIMessage &message) {
    return Widget::onMessage(message);
}

glm::vec2 Menu::getPreferredSize() {
    return label_->getPreferredSize();
}

