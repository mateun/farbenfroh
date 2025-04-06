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
    closing_icon_texture_ = std::make_shared<Texture>("../assets/green_x_16.png");

}

FloatingWindow::~FloatingWindow() {
}

void FloatingWindow::draw(float depth) {
    float window_bg_depth = 1.0f;

    MeshDrawData mdd;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetRoundedRectShader(false);

    mdd.viewPortDimensions =  global_size_;
    mdd.setViewport = true;
    mdd.viewport = {global_origin_.x,  global_origin_.y, global_size_.x, global_size_.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", bg_gradient_start_}};
    mdd.location = {0, 0, window_bg_depth};
    mdd.color = bg_gradient_start_;
    mdd.scale = {global_size_.x, global_size_.y, 1};
    //mdd.scale = {splitterPosition_.x - splitterSize - 1, size_.y - 5, 1};
    Renderer::drawWidgetMeshDeferred(mdd, this);

    // We always render a window header to enable closing of the window:
    mdd.location = {0,  global_size_.y - 20, window_bg_depth + 0.01};
    mdd.scale = {global_size_.x, 20, 1};
    mdd.color = {0.6, 0.6, 0.61, 1};
    Renderer::drawWidgetMeshDeferred(mdd, this);

    renderCloseButtonHover(window_bg_depth + 0.02);

    // Render the close button
    mdd.location = {global_size_.x - 20, global_size_.y - 20 + 2, window_bg_depth + 0.03};
    mdd.scale = {16, 16, 1};
    mdd.texture= closing_icon_texture_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(true);
    Renderer::drawWidgetMeshDeferred(mdd, this);


}

void FloatingWindow::close() {
    getApplication()->removeFloatingWindow(std::dynamic_pointer_cast<FloatingWindow>(shared_from_this()));
}

void FloatingWindow::renderCloseButtonHover(float depth) {
    if (!hovering_close_button_) return;

    float underLayWidth = 24;
    float underLayHeight = 20;
    MeshDrawData mddUnderlay;
    mddUnderlay.mesh = quadMesh_;
    mddUnderlay.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);
    mddUnderlay.viewPortDimensions =  {underLayWidth, underLayHeight};
    mddUnderlay.setViewport = true;

    mddUnderlay.viewport = {global_origin_.x + global_size_.x - underLayWidth,  global_origin_.y + global_size_.y - underLayHeight, mddUnderlay.viewPortDimensions.value().x , mddUnderlay.viewPortDimensions.value().y};
    glm::vec4 col = glm::vec4{0.6, 0.04, 0.04, 1};
    mddUnderlay.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", col}};
    mddUnderlay.color = col;
    mddUnderlay.scale = {underLayWidth, underLayHeight , 1};
    mddUnderlay.location = {0, 0, depth};
    mddUnderlay.debugInfo = "floating_window_" + id_ + " close-button-underlay";
    Renderer::drawWidgetMeshDeferred(mddUnderlay, this);
}

MessageHandleResult FloatingWindow::onMessage(const UIMessage &message) {

    if (message.type == MessageType::MouseMove) {
        if (message.mouseMoveMessage.x >= global_origin_.x + global_size_.x - 20 && message.mouseMoveMessage.y > global_origin_.y + global_size_.y - 20) {
            hovering_close_button_ = true;
        } else {
            hovering_close_button_ = false;
        }

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
        glm::vec2 pt = getApplication()->getCurrentMousePos();
        offset_from_pivot_ = glm::vec2(pt.x, pt.y) - global_origin_;

        if (hover_focus_) {
            if (pt.x >= global_origin_.x + global_size_.x - 20 && pt.y > global_origin_.y + global_size_.y - 20) {
                close();
                return MessageHandleResult{true, "", true};
            }
        }

        mouse_down_ = true;
        //std::cout << "offset_from_pivot_: " << std::to_string(offset_from_pivot_.x) << "/" << std::to_string(offset_from_pivot_.y) << std::endl;

    }

    if (message.type == MessageType::MouseUp) {
        mouse_down_ = false;
        last_mouse_pos_ = {0, 0};
    }

    if (hover_focus_) {
        return MessageHandleResult{true, "", true};
    } else {
        return MessageHandleResult{true, "", false};
    }

}

void FloatingWindow::setHoverFocus(std::shared_ptr<Widget> prevFocusHolder) {
    //std::cout << "FloatingWindow::setHoverFocus" << std::endl;
    hover_focus_ = true;
}

void FloatingWindow::removeHoverFocus() {
    hover_focus_ = false;
}
