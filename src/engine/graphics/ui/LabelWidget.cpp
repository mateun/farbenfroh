//
// Created by mgrus on 27.03.2025.
//

#include "LabelWidget.h"

#include <engine/graphics/Geometry.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/TrueTypeTextRenderer.h>

LabelWidget::LabelWidget(const std::string &text, const std::shared_ptr<TrueTypeFont>& font): text_(text),
    font_(font) {
    quadMesh = gru::Geometry::createQuadMesh(PlanePivot::bottomleft);
    textRenderer_ = std::make_shared<TrueTypeTextRenderer>(font_);

}

void LabelWidget::draw(Camera* camera) {

    glm::vec2 dim1;
    auto textMesh = textRenderer_->renderText(text_, &dim1);

    MeshDrawData mdd;
    mdd.mesh = textMesh.get();
    mdd.texture = font_->getAtlas().get();
    mdd.location = {origin_x, origin_y, -1};
    mdd.scale = {1, 1, 1};
    mdd.camera = camera;
    mdd.shader = getDefaultWidgetShader().get();
    Renderer::drawMesh(mdd);



}

void LabelWidget::setText(const std::string &text) {
    text_ = text;
}

