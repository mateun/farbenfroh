//
// Created by mgrus on 30.03.2025.
//

#include "ButtonWidget.h"

#include <iostream>
#include <engine/graphics/Application.h>
#include <engine/graphics/Geometry.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/ui/UIMessage.h>

#include "LabelWidget.h"
#include "MessageHandleResult.h"
#include <engine/graphics/ui/RoundedRect.h>

ButtonWidget::ButtonWidget(std::shared_ptr<Texture> texture) {
    texture_ = texture;
}

ButtonWidget::ButtonWidget(const std::string& text, const std::shared_ptr<TrueTypeFont>& font) {
    font_ = font;
    round_rect_ = std::make_shared<RoundedRect>(8);
    label_widget_ = std::make_shared<LabelWidget>(text, font);
    text_ = text;
}

void ButtonWidget::draw(float depth) {



    if (texture_) {
        drawTexturedButton(depth);
    } else {
        drawTextButton(depth);
    }


}

MessageHandleResult ButtonWidget::onMessage(const UIMessage &message) {

    switch (message.type) {
        case MessageType::MouseMove: {
            hover_ = checkMouseOver(message.mouseMoveMessage.x, message.mouseMoveMessage.y);
            if (hover_) {
                return MessageHandleResult {true, "", true};
            }
            break;
        }
        case MessageType::MouseDown: {
            if (message.num == last_processed_message_num_) {
                return MessageHandleResult {false, "", true};
            }
            setLastProcessedMessage(message.num);

            if ((hover_focus_ || hover_) && enabled_) {
                std::cout << "button widget clicked for message: " << message.num << std::endl;
                for (auto ab : action_callbacks_) {
                    ab(shared_from_this());
                }
                return MessageHandleResult {false, "", true};
            }
        }
    }

    return MessageHandleResult {false, "", false};
}

glm::vec2 ButtonWidget::getPreferredSize() {
    return {32, 32};
}

void ButtonWidget::setTexture(std::shared_ptr<Texture> tex) {
    texture_ = tex;
}

void ButtonWidget::setHoverFocus(std::shared_ptr<Widget> prevFocusHolder) {
    if (enabled_) {
        hover_focus_ = true;
    }
}

void ButtonWidget::removeHoverFocus() {
    hover_focus_ = false;
}



void ButtonWidget::setLastProcessedMessage(uint64_t lastProcessedMessage) {
    last_processed_message_num_ = lastProcessedMessage;
}

void ButtonWidget::drawTexturedButton(float depth) {

    if (hover_) {
        drawHoverUnderlay(depth);
    }

    MeshDrawData mdd;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(texture_ != nullptr );
    mdd.viewPortDimensions =  global_size_;
    mdd.setViewport = true;
    mdd.viewport = {global_origin_.x,  global_origin_.y, global_size_.x, global_size_.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.5, 0.4, 0.0, 1}}};
    mdd.texture = texture_;
    mdd.scale = {global_size_.x, global_size_.y, 1};
    mdd.location = {0, 0, depth + 0.5};
    mdd.debugInfo = "button_" + id_;

    Renderer::drawWidgetMeshDeferred(mdd, this);
}

void ButtonWidget::drawTextButton(float depth) {

    // Background rect to hold the text label
    if (hover_) {
        round_rect_->setBgColor({.1, .1, .4, 1});
    } else {
        round_rect_->setBgColor({.1, .1, .6, 1});
    }

    if (!enabled_) {
        round_rect_->setBgColor({0.05, 0.05, 0.05, 1});
    }

    round_rect_->setPreferredSize({global_size_.x - 12, 30});
    round_rect_->setSize(round_rect_->getPreferredSize());
    round_rect_->setOrigin(origin() + glm::vec2(0, 0));
    round_rect_->draw(depth + 0.02);

    // The label with the current text
    label_widget_->setSize(round_rect_->getPreferredSize());
    // Center the text in the button
    auto textWidth = label_widget_->calculateSizeForText(text_).dimensions.x;
    label_widget_->setOrigin(origin() +  glm::vec2(size().x/2 - textWidth/2, 6));
    label_widget_->setTextColor({1, 1, 1, 1});  // TODO text color
    label_widget_->draw(depth + .03);
}

void ButtonWidget::drawHoverUnderlay(float depth) {
    MeshDrawData mddUnderlay;
    mddUnderlay.mesh = quadMesh_;
    mddUnderlay.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);
    mddUnderlay.viewPortDimensions =  global_size_;
    mddUnderlay.setViewport = true;
    float underLayWidth = global_size_.x + 2;
    mddUnderlay.viewport = {global_origin_.x-2,  global_origin_.y-2, underLayWidth , global_size_.y};
    mddUnderlay.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.035, 0.143, 0.298, 0.45}}};
    mddUnderlay.color = glm::vec4{0.035, 0.143, 0.298, 0.45};
    mddUnderlay.scale = {underLayWidth, global_size_.y , 1};
    mddUnderlay.location = {0, 1, depth + 0.01};
    mddUnderlay.debugInfo = id_;
    Renderer::drawWidgetMeshDeferred(mddUnderlay, this);
}

void ButtonWidget::enable() {
    enabled_ = true;
}

void ButtonWidget::disable() {
    enabled_ = false;
}



