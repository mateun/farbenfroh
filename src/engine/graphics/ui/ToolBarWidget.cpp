//
// Created by mgrus on 30.03.2025.
//

#include "ToolBarWidget.h"

#include <engine/graphics/Application.h>
#include <engine/graphics/Geometry.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/ui/MessageHandleResult.h>

ToolBarWidget::ToolBarWidget() {
    quadMesh_ = gru::Geometry::createQuadMesh(PlanePivot::bottomleft);
    h_box_layout_ = std::make_shared<HBoxLayout>();
    h_box_layout_->setMarginHorizontal(10);
}

void ToolBarWidget::draw(float depth) {

    h_box_layout_->apply(this);

    MeshDrawData mdd;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);

    mdd.viewPortDimensions =  global_size_;
    mdd.setViewport = true;
    mdd.viewport = {global_origin_.x,  global_origin_.y, global_size_.x, global_size_.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.01, 0.01, 0.01, 1}}};
    mdd.color = {0.02, 0.02,0.02, 1};
    mdd.location = {0, 0, depth + 0.01};
    mdd.scale = {global_size_.x, global_size_.y, 1};
    mdd.debugInfo = "toolbar_background";
    Renderer::drawWidgetMeshDeferred(mdd, this);

    // Now render all ButtonWidgets
    for (auto b : children()) {
        b->draw(depth + 0.02);
    }

}

MessageHandleResult ToolBarWidget::onMessage(const UIMessage &message) {
    return Widget::onMessage(message);
}

glm::vec2 ToolBarWidget::getPreferredSize() {
    return {global_size_.x, 40};
}
