//
// Created by mgrus on 27.03.2025.
//

#include "LabelWidget.h"

#include <engine/graphics/Application.h>
#include <engine/graphics/Geometry.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/TrueTypeTextRenderer.h>

LabelWidget::LabelWidget(const std::string &text, const std::shared_ptr<TrueTypeFont>& font): text_(text),
    font_(font) {
    quadMesh = gru::Geometry::createQuadMesh(PlanePivot::bottomleft);
    textRenderer_ = std::make_shared<TrueTypeTextRenderer>(font_);

}

void LabelWidget::draw() {

    glm::vec2 dim1;
    auto textMesh = textRenderer_->renderText(text_, &dim1);

    MeshDrawData mdd;
    mdd.mesh = textMesh;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader();
    //mdd.camera = getApplication()->getRenderBackend()->getOrthoCameraForViewport(origin_.x, origin_.y, size_.x, size_.y).get();
    auto cam = new Camera();
    cam->updateLocation({0, 0, -2});
    cam->updateLookupTarget({0, 0, -1});
    //cam->setProjectionMetrics(0, size_.x, 0, size_.y);
    mdd.camera_shared = std::shared_ptr<Camera>(cam);
    mdd.viewPortDimensions =  size_;
    mdd.setViewport = true;
    mdd.viewport = {origin_.x, origin_.y, size_.x, size_.y};
    mdd.texture = font_->getAtlas().get();
    mdd.location = {origin_.x, origin_.y, -1};
    mdd.scale = {1, 1, 1};
    Renderer::drawWidgetMeshDeferred(mdd, this);


}

void LabelWidget::setText(const std::string &text) {
    text_ = text;
}

glm::vec2 LabelWidget::getPreferredSize() {
    // TODO make dependent on the text size
    return {200, 50};
}

glm::vec2 LabelWidget::getMinSize() {
    // TODO make dep. on text
    return {80, 50};
}

glm::vec2 LabelWidget::getMaxSize() {
    // TODO make dep. on text
    return {300, 50};
}

