//
// Created by mgrus on 02.04.2025.
//

#include "FloatingWindow.h"

#include <iostream>
#include <engine/graphics/Application.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>

#include "MessageHandleResult.h"

FloatingWindow::FloatingWindow() {
}

FloatingWindow::~FloatingWindow() {
}

void FloatingWindow::draw(float depth) {

    MeshDrawData mdd;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);

    mdd.viewPortDimensions =  global_size_;
    mdd.setViewport = true;
    mdd.viewport = {global_origin_.x,  global_origin_.y, global_size_.x, global_size_.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{1.0, 0.0, 0.0, 1}}};
    mdd.location = {0, 0, 1};
    mdd.color = glm::vec4{1.0f, 0.0f, 0.0f, 1};
    mdd.scale = {global_size_.x, global_size_.y, 1.0f};
    //mdd.scale = {splitterPosition_.x - splitterSize - 1, size_.y - 5, 1};
    Renderer::drawWidgetMeshDeferred(mdd, this);


}

MessageHandleResult FloatingWindow::onMessage(const UIMessage &message) {


    if (message.type == MessageType::MouseMove) {
        if (hover_focus_ && mouse_down_) {
            glm::vec2 netMovement = glm::vec2(message.mouseMoveMessage.x, message.mouseMoveMessage.y) - last_mouse_pos_;
            if (last_mouse_pos_.x == 0 && last_mouse_pos_.y == 0) {
                netMovement = {0, 0};
            }

            std::cout << "netMovement: " << std::to_string(netMovement.x) << " " << std::to_string(netMovement.y) << std::endl;
            setOrigin(global_origin_ + netMovement);
            last_mouse_pos_ = glm::vec2(message.mouseMoveMessage.x, message.mouseMoveMessage.y) ;
        }
    }

    if (message.type == MessageType::MouseDown) {
        mouse_down_ = true;


        glm::vec2 pt = getApplication()->getCurrentMousePos();
        offset_from_pivot_ = glm::vec2(pt.x, pt.y) - global_origin_;
        std::cout << "offset_from_pivot_: " << std::to_string(offset_from_pivot_.x) << "/" << std::to_string(offset_from_pivot_.y) << std::endl;

    }

    if (message.type == MessageType::MouseUp) {
        mouse_down_ = false;
        last_mouse_pos_ = {0, 0};
    }

    return MessageHandleResult{true, "", false};
}

void FloatingWindow::setHoverFocus(std::shared_ptr<Widget> prevFocusHolder) {
    //std::cout << "FloatingWindow::setHoverFocus" << std::endl;
    hover_focus_ = true;
}

void FloatingWindow::removeHoverFocus() {
    hover_focus_ = false;
}
