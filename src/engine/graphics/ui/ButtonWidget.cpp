//
// Created by mgrus on 30.03.2025.
//

#include "ButtonWidget.h"

#include <engine/graphics/Application.h>
#include <engine/graphics/Geometry.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>

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
        mdd.color = glm::vec4{.2, 0.2, 0.1, 1.0};
        mdd.scale = {global_size_.x * 0.9, global_size_.y *.9, 1};
    } else {
        mdd.scale = {global_size_.x, global_size_.y, 1};
        mdd.color = {0.5, 0.4,0.0, 1};
    }
    mdd.texture = texture_;
    mdd.location = {0, 0, -1};
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
        }
        case MessageType::MouseDown: {
            break;
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
