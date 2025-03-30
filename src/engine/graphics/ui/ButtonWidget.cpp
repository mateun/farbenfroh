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
    quadMesh_ = gru::Geometry::createQuadMesh(PlanePivot::bottomleft);
}

void ButtonWidget::draw() {

    MeshDrawData mdd;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(texture_ != nullptr );

    auto cam = std::make_shared<Camera>(CameraType::Ortho);
    cam->updateLocation({0, 0, 2});
    cam->updateLookupTarget({0, 0, -1});
    mdd.camera_shared = cam;
    mdd.viewPortDimensions =  size_;
    mdd.setViewport = true;
    mdd.viewport = {origin_.x,  origin_.y, size_.x, size_.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.5, 0.4, 0.0, 1}}};
    if (hover_) {
        mdd.color = glm::vec4{.2, 0.2, 0.1, 1.0};
        mdd.scale = {size_.x * 0.9, size_.y *.9, 1};
    } else {
        mdd.scale = {size_.x, size_.y, 1};
        mdd.color = {0.5, 0.4,0.0, 1};
    }
    mdd.texture = texture_;
    mdd.location = {0, 0, -1.3};

    Renderer::drawWidgetMeshDeferred(mdd, this);

}

MessageHandleResult ButtonWidget::onMessage(const UIMessage &message) {

    switch (message.type) {
        case MessageType::MouseMove: {
            // TODO create helper for this mouse coordinate checking.
            if (message.mouseMoveMessage.x >= origin_.x && message.mouseMoveMessage.x <= size_.x &&
                message.mouseMoveMessage.y <= origin_.y + size_.y) {
                hover_ = true;
            } else {
                hover_ = false;
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
