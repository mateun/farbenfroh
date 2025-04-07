//
// Created by mgrus on 02.04.2025.
//

#include "FloatingWindow.h"

#include <iostream>
#include <engine/graphics/Application.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>

#include "MessageHandleResult.h"
#include "RoundedRect.h"

FloatingWindow::FloatingWindow() {
    closing_icon_texture_ = std::make_shared<Texture>("../assets/green_x_16.png");
    body_widget_ = std::make_shared<Widget>();
    body_widget_->setId("floating_window_" + id_ + "_body");
    main_window_rect_ = std::make_shared<RoundedRect>(8);
    main_window_rect_->setId("floating_window_" + id_ + "_outer_rect");


}

FloatingWindow::~FloatingWindow() {
}

float FloatingWindow::getCloseAreaLeft() {
    return global_origin_.x + global_size_.x - getCloseAreaUnderLayDimensions().x - 12;
}

float FloatingWindow::getCloseAreaBottom() {
    return global_origin_.y + global_size_.y - getCloseAreaUnderLayDimensions().y - 8;
}

void FloatingWindow::draw(float depth) {
    float window_bg_depth = 1.0f;





    main_window_rect_->setSize(global_size_);
    main_window_rect_->setBgColor({0.1, 0.1, 0.1, 1});
    main_window_rect_->setOrigin(global_origin_);
    main_window_rect_->draw(window_bg_depth);


    // We always render a window header to enable closing of the window:
    float header_height = 20;
    float header_width = global_size_.x - 16;
    MeshDrawData mdd;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetRoundedRectShader(false);
    mdd.viewPortDimensions =  global_size_;
    mdd.setViewport = true;
    mdd.viewport = {global_origin_.x,  global_origin_.y, global_size_.x, global_size_.y};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", bg_gradient_start_}};
    mdd.color = bg_gradient_start_;
    mdd.location = {4,  global_size_.y - header_height - 8, window_bg_depth + 0.01};
    mdd.scale = {header_width, header_height, 1};
    mdd.color = {0.6, 0.6, 0.61, 1};
    //Renderer::drawWidgetMeshDeferred(mdd, this);


    renderCloseButtonHover(window_bg_depth + 0.02);

    // Render the close button
    float icon_size = 16;
    mdd.location = {global_size_.x - icon_size - 16, global_size_.y - icon_size - 2 -8 , window_bg_depth + 0.03};
    mdd.scale = {icon_size, icon_size, 1};
    mdd.texture= closing_icon_texture_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(true);
    Renderer::drawWidgetMeshDeferred(mdd, this);

    // Render the window body.
    // First we apply the layout which was set to the window.
    // Use an AreaLayout in case none was set.
    body_widget_->setBgColor({0.1, 0.1, 0.1, 1});
    body_widget_->setSize(global_size_ - glm::vec2(8, header_height + 8) );
    body_widget_->setOrigin(origin() + glm::vec2(0, 0));

    body_widget_->clearChildren();
    for (auto c : children_) {
        body_widget_->addChild(c);
    }
    if (layout_) {
        body_widget_->setLayout(layout_);
    } else {
        layout_ = std::make_shared<AreaLayout>();
        body_widget_->setLayout(layout_);
    }
    body_widget_->draw(depth);


    glDisable(GL_SCISSOR_TEST);


}

void FloatingWindow::close() {
    getApplication()->removeFloatingWindow(std::dynamic_pointer_cast<FloatingWindow>(shared_from_this()));
}

void FloatingWindow::renderCloseButtonHover(float depth) {
    if (!hovering_close_button_) return;

    float underLayWidth = getCloseAreaUnderLayDimensions().x;
    float underLayHeight = getCloseAreaUnderLayDimensions().y;
    MeshDrawData mddUnderlay;
    mddUnderlay.mesh = quadMesh_;
    mddUnderlay.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);
    mddUnderlay.viewPortDimensions =  {underLayWidth, underLayHeight};
    mddUnderlay.setViewport = true;

    mddUnderlay.viewport = {getCloseAreaLeft(),  getCloseAreaBottom(), mddUnderlay.viewPortDimensions.value().x , mddUnderlay.viewPortDimensions.value().y};
    glm::vec4 col = glm::vec4{0.6, 0.04, 0.04, 1};
    mddUnderlay.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", col}};
    mddUnderlay.color = col;
    mddUnderlay.scale = {underLayWidth, underLayHeight , 1};
    mddUnderlay.location = {0, 0, depth};
    mddUnderlay.debugInfo = "floating_window_" + id_ + " close-button-underlay";
    Renderer::drawWidgetMeshDeferred(mddUnderlay, this);
}

bool FloatingWindow::mouseOverCloseButton(int mouse_x, int mouse_y) {
    return mouse_x >= getCloseAreaLeft() &&
            mouse_x <= (getCloseAreaLeft() + getCloseAreaUnderLayDimensions().x) &&
            mouse_y > getCloseAreaBottom() &&
            mouse_y < (getCloseAreaBottom() + getCloseAreaUnderLayDimensions().y);
}

MessageHandleResult FloatingWindow::onMessage(const UIMessage &message) {

    if (message.type == MessageType::MouseMove) {
        if (mouseOverCloseButton(message.mouseMoveMessage.x, message.mouseMoveMessage.y)) {
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
            if (mouseOverCloseButton(pt.x, pt.y)) {
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

glm::vec2 FloatingWindow::getCloseAreaUnderLayDimensions() {
    float underLayWidth = 24;
    float underLayHeight = 20;
    return glm::vec2(underLayWidth, underLayHeight);

}
