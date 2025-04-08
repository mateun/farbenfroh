//
// Created by mgrus on 07.04.2025.
//

#include "TextInput.h"

#include <iostream>
#include <engine/game/Timing.h>
#include <engine/graphics/Application.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/ui/UIMessage.h>

#include "LabelWidget.h"
#include "MessageHandleResult.h"
#include "RoundedRect.h"

TextInput::TextInput(const std::string &initialText, const std::shared_ptr<TrueTypeFont> &font) : initial_text_(initialText) , font_(font) {
    label_widget_ = std::make_shared<LabelWidget>(initialText, font_);
    label_widget_->setId("text_input_" + id_ + "_inner_label");
    input_field_ = std::make_shared<RoundedRect>(10);
    input_field_->setId("text_input_" + id_ + "_input_field");

    // Indicates in front of which char the cursor shall be positioned:
    char_cursor_pos_ = text_.size();

}

float TextInput::charCursorToPixelPos() {
    std::string sub = text_.substr(0, char_cursor_pos_ );
    auto dim = label_widget_->calculateSizeForText(sub);
    return dim.dimensions.x + 4;
}

glm::vec2 TextInput::getPreferredSize() {
    auto labelPref = label_widget_->getPreferredSize();
    return glm::vec2(labelPref.x, font_ ->getLineHeight() * 2);

}

void TextInput::setTextColor(glm::vec4 color) {
    text_color_ = color;
}

void TextInput::addTextChangeListener(std::function<void(std::shared_ptr<TextInput>)> listener) {
    text_change_listeners_.emplace_back(listener);
}


void TextInput::setHoverFocus(std::shared_ptr<Widget> prevFocusHolder) {
    hover_focus_ = true;
    std::cout << "text input got hover focus" << std::endl;
}

void TextInput::removeHoverFocus() {
    hover_focus_ = false;
}

std::string TextInput::getText() {
    return text_;
}


void TextInput::draw(float depth) {

    if (hover_focus_) {
        getApplication()->setSpecialCursor(CursorType::TextEdit);
    } else {
        getApplication()->unsetSpecialCursor();
    }

    // We must set a valid "virtual" z-value for the textInput widget itself.
    // Normally this gets set during the Renderer::drawDeferred(mdd) call,
    // but we do not directly render ourselves as TextInput here, so we set it directly and
    // are found by the focus manager.
    setZValue(depth + 0.01);

    drawCursor(depth + 0.04);


    // Background rect to hold the text label
    input_field_->setBgColor({.9, .9, .9, 1});
    input_field_->setPreferredSize({global_size_.x - 12, 30});
    input_field_->setSize(input_field_->getPreferredSize());
    input_field_->setOrigin(origin() + glm::vec2(0, 0));
    input_field_->draw(depth + 0.02);

    // The label with the current text
    label_widget_->setSize(input_field_->getPreferredSize());
    label_widget_->setOrigin(origin() + glm::vec2(6, 6));
    label_widget_->setTextColor(text_color_);
    label_widget_->draw( depth + .03);

    // Draw marked text (nice-to-have...)

}

void TextInput::drawCursor(float depth) {
    blink_timer_ += Timing::lastFrameTimeInSeconds();
    if (blink_timer_ > 0.530) {
        blink_timer_ready_ = !blink_timer_ready_;
        blink_timer_ = 0;
    }

    if (!render_cursor_ || !blink_timer_ready_) {
        return;
    }

    glm::vec2 cursorDim = {1, input_field_->size().y - 8};
    glm::vec2 cursorOrigin = {input_field_->origin().x + 2 + charCursorToPixelPos()  ,  input_field_->origin().y + 4};
    glm::vec4 cursorColor = {0.01, 0.01, 0.01, 1};
    MeshDrawData mdd;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);
    mdd.viewPortDimensions =  cursorDim;
    mdd.setViewport = true;
    mdd.viewport = {cursorOrigin.x, cursorOrigin.y, cursorDim.x , cursorDim.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", glm::vec2(cursorDim.x, cursorDim.y)},
                            ShaderParameter{"viewPortOrigin", cursorOrigin},
                            ShaderParameter{"gradientTargetColor", cursorColor}};
    mdd.color = cursorColor;
    mdd.scale = {cursorDim.x, cursorDim.y , 1};
    mdd.location = {0, 0 , depth  + 0.5};
    mdd.debugInfo = "cursor_for_text_input_" + id_;
    Renderer::drawWidgetMeshDeferred(mdd, this);
}

void TextInput::invokeTextListenerCallbacks() {
    for (auto& tl: text_change_listeners_) {
        tl(std::dynamic_pointer_cast<TextInput>(shared_from_this()));
    }
}

MessageHandleResult TextInput::onMessage(const UIMessage &message) {


    if (message.type == MessageType::KeyDown && render_cursor_) {
        if (message.num != prev_message_num_) {
            if (message.keyboardMessage.key == VK_LEFT) {

                char_cursor_pos_ --;
                if (char_cursor_pos_ < 0) {
                    char_cursor_pos_ = 0;
                }
            }

            if (message.keyboardMessage.key == VK_RIGHT) {

                char_cursor_pos_ ++;
                if (char_cursor_pos_ > text_.size()) {
                    char_cursor_pos_ = text_.size();
                }
            }
            if (message.keyboardMessage.key == VK_BACK) {
                if ( (char_cursor_pos_ ) > 0) {
                    char_cursor_pos_--;
                    text_.erase(char_cursor_pos_, 1);
                    label_widget_->setText(text_);
                    invokeTextListenerCallbacks();
                }
            }
            prev_message_num_ = message.num;
        }
    }

    if (message.type == MessageType::Character && render_cursor_ ) {
        if (message.num != prev_message_num_ && message.character != '\b') {
            text_.insert(char_cursor_pos_, 1, message.character);
            char_cursor_pos_++;
            label_widget_->setText(text_);
            prev_message_num_ = message.num;
            invokeTextListenerCallbacks();
            return MessageHandleResult {false, "", true};
        }
    }

    if (message.type == MessageType::MouseDown) {
        if (hover_focus_) {
            render_cursor_ = true;
            std::cout << "Textinput "<< id_ << " got click focus!" << std::endl;
        } else {
            // We assume clicked somewhere outside
            render_cursor_ = false;
        }

    }

    return MessageHandleResult {false, "", false};

}


