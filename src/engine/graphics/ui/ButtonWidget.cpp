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

#include "MessageHandleResult.h"

ButtonWidget::ButtonWidget() {
}

void ButtonWidget::draw(float depth) {

    MeshDrawData mdd;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(texture_ != nullptr );


    mdd.viewPortDimensions =  global_size_;
    mdd.setViewport = true;
    mdd.viewport = {global_origin_.x,  global_origin_.y, global_size_.x, global_size_.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.5, 0.4, 0.0, 1}}};
    if (hover_) {
        //mdd.color = glm::vec4{.2, 0.2, 0.1, 1.0};
        //mdd.scale = {global_size_.x * 0.9, global_size_.y *.9, 1};

        // Draw a background quad on hovering.
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

    } else {
        //mdd.scale = {global_size_.x, global_size_.y, 1};
        //mdd.color = {0.5, 0.4,0.0, 1};
    }
    mdd.scale = {global_size_.x, global_size_.y, 1};
    mdd.texture = texture_;
    mdd.location = {0, 0, depth + 0.5};
    mdd.debugInfo = "button";

    Renderer::drawWidgetMeshDeferred(mdd, this);

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

            if (hover_focus_) {
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
    hover_focus_ = true;
}

void ButtonWidget::removeHoverFocus() {
    hover_focus_ = false;
}

void ButtonWidget::setLastProcessedMessage(uint64_t lastProcessedMessage) {
    last_processed_message_num_ = lastProcessedMessage;
}




