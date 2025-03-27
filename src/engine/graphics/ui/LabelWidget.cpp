//
// Created by mgrus on 27.03.2025.
//

#include "LabelWidget.h"

#include <engine/graphics/Geometry.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/TrueTypeTextRenderer.h>

LabelWidget::LabelWidget(const std::string &text): text_(text) {
    quadMesh = gru::Geometry::createQuadMesh(PlanePivot::bottomleft);

    textRenderer_ = std::make_shared<TrueTypeTextRenderer>("../assets/calibri.ttf");

}

void LabelWidget::draw(Camera* camera) {

    auto textMesh = textRenderer_->renderText("Hello world! with great 12334$%");


    MeshDrawData mdd;
    mdd.mesh = textMesh.get();
    mdd.texture = textRenderer_->getFontAtlas().get();
    //mdd.color = glm::vec4{1, 0,1, 1};
    mdd.location = {10, 300, -1};
    mdd.scale = {1, 1, 1};
    mdd.camera = camera;
    mdd.shader = getDefaultWidgetShader().get();
    Renderer::drawMesh(mdd);



}
