//
// Created by mgrus on 27.03.2025.
//

#include "LabelWidget.h"

#include <engine/graphics/Application.h>
#include <engine/graphics/ErrorHandling.h>
#include <engine/graphics/Geometry.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/TrueTypeTextRenderer.h>

#include "MessageHandleResult.h"

LabelWidget::LabelWidget(const std::string &text, const std::shared_ptr<TrueTypeFont>& font): text_(text),
                                                                                              font_(font) {
    quadMesh = gru::Geometry::createQuadMesh(PlanePivot::bottomleft);
    textRenderer_ = std::make_shared<TrueTypeTextRenderer>(font_);

}

void LabelWidget::draw(float depth) {
    glm::vec2 dim1;

    auto textMesh = textRenderer_->renderText(text_, &dim1);

    MeshDrawData mdd;
    mdd.mesh = textMesh;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultTextShader();


    mdd.viewPortDimensions =  global_size_;
    mdd.setViewport = true;
    mdd.viewport = {global_origin_.x,  global_origin_.y, global_size_.x, global_size_.y};
    mdd.texture = font_->getAtlas();
    mdd.location = {0, abs(dim1.y), depth};
    mdd.scale = {1, 1, 1};
    Renderer::drawWidgetMeshDeferred(mdd, this);

}

void LabelWidget::setText(const std::string &text) {
    text_ = text;
}

glm::vec2 LabelWidget::getPreferredSize() {
    auto textSize = textRenderer_->calculateTextDimension(text_);
    return {textSize.x, textSize.y * 3};
}

glm::vec2 LabelWidget::getMinSize() {
    return getPreferredSize();
}

glm::vec2 LabelWidget::getMaxSize() {
    auto dim = textRenderer_->calculateTextDimension(text_);
    return {dim.x + 20, dim.y + 20};
}

MessageHandleResult LabelWidget::onMessage(const UIMessage &message) {
    return Widget::onMessage(message);
}


