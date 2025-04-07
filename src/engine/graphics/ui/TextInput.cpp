//
// Created by mgrus on 07.04.2025.
//

#include "TextInput.h"

#include <engine/graphics/Application.h>

#include "LabelWidget.h"
#include "MessageHandleResult.h"
#include "RoundedRect.h"

TextInput::TextInput(const std::string &initialText, const std::shared_ptr<TrueTypeFont> &font) : text_(initialText) , font_(font) {
    label_widget_ = std::make_shared<LabelWidget>(initialText, font_);
}

TextInput::~TextInput() {
}

glm::vec2 TextInput::getPreferredSize() {
    return label_widget_->getPreferredSize() + glm::vec2(10, 16);
}

void TextInput::setTextColor(glm::vec4 color) {
    text_color_ = color;
}


void TextInput::draw(float depth) {
    // Background rect to hold the text label
    auto rr = std::make_shared<RoundedRect>(10);
    rr->setBgColor({.9, .9, .9, 1});
    rr->setPreferredSize({global_size_.x - 12, 30});
    rr->setSize(rr->getPreferredSize());
    rr->setOrigin(origin() + glm::vec2(0, 0));
    rr->draw(depth + 0.01);

    // The label with the current text
    label_widget_->setSize(rr->getPreferredSize());
    label_widget_->setOrigin(origin() + glm::vec2(6, 6));
    label_widget_->setTextColor(text_color_);
    label_widget_->draw( depth + .02);

    // The cursor

    // Any marked text

}

MessageHandleResult TextInput::onMessage(const UIMessage &message) {

    if (message.type == MessageType::Character) {
        text_ += message.character;
        return MessageHandleResult {false, "", true};
    }

    return MessageHandleResult {false, "", false};

}
