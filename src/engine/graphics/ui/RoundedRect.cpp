//
// Created by mgrus on 07.04.2025.
//

#include "RoundedRect.h"

#include <engine/graphics/Application.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/ui/MessageHandleResult.h>

RoundedRect::RoundedRect(float cornerRadius): corner_radius_(cornerRadius) {
}

void RoundedRect::draw(float depth) {
    MeshDrawData mdd;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetRoundedRectShader(false);
    mdd.viewPortDimensions =  global_size_;
    mdd.setViewport = true;
    mdd.viewport = {global_origin_.x,  global_origin_.y, global_size_.x, global_size_.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_},
                            ShaderParameter{"viewPortOrigin", origin()},
                            ShaderParameter{"gradientTargetColor", bg_gradient_start_},
                            ShaderParameter{"radius", 8.0f},
                            ShaderParameter{"rectSize", global_size_}};
    mdd.location = {0, 0, depth};
    mdd.color = bg_gradient_start_;
    mdd.scale = {global_size_.x, global_size_.y, 1};
    //mdd.scale = {splitterPosition_.x - splitterSize - 1, size_.y - 5, 1};
    Renderer::drawWidgetMeshDeferred(mdd, this);
}


