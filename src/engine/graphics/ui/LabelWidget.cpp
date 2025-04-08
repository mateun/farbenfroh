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
    if (text_.empty()) return;

    TextDimensions text_dimensions;

    auto textMesh = textRenderer_->renderText(text_, &text_dimensions);

    MeshDrawData mdd;
    mdd.mesh = textMesh;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultTextShader();

    mdd.viewPortDimensions =  global_size_;
    mdd.setViewport = true;
    mdd.viewport = {global_origin_.x,  global_origin_.y, global_size_.x, global_size_.y};
    mdd.shaderParameters = {ShaderParameter{"textColor", text_color_}};
    mdd.texture = font_->getAtlas();
    //mdd.location = {0, abs(dim1.y), depth};
    mdd.location = {0, -font_->getMaxDescent(), depth};
    mdd.scale = {1, 1, 1};
    Renderer::drawWidgetMeshDeferred(mdd, this);

}

void LabelWidget::setText(const std::string &text) {
    text_ = text;
}

void LabelWidget::setTextColor(glm::vec4 text_color) {
    text_color_ = text_color;
}

TextDimensions LabelWidget::calculateSizeForText(const std::string &str) {
    return textRenderer_->calculateTextDimension(str);
}

glm::vec2 LabelWidget::getPreferredSize() {
    auto textSize = textRenderer_->calculateTextDimension(text_);
    return {textSize.dimensions.x, textSize.dimensions.y * 2};
}

glm::vec2 LabelWidget::getMinSize() {
    return getPreferredSize();
}

glm::vec2 LabelWidget::getMaxSize() {
    auto text_dim = textRenderer_->calculateTextDimension(text_);
    return {text_dim.dimensions.x + 20, text_dim.dimensions.y + 20};
}

MessageHandleResult LabelWidget::onMessage(const UIMessage &message) {
    return {true, "", false};
}


