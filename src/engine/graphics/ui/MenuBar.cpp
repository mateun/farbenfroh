//
// Created by mgrus on 31.03.2025.
//

#include "MenuBar.h"

#include <engine/graphics/ui/Menu.h>
#include <engine/graphics/Application.h>
#include <engine/graphics/ui/MessageHandleResult.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>

MenuBar::MenuBar() {
}

void MenuBar::addMenu(std::shared_ptr<Menu> menu) {
    menus_.push_back(menu);
    children_.push_back(menu);
}

void MenuBar::draw() {

    // First we draw a background panel:
    MeshDrawData mdd;
    mdd.mesh = quadMesh_;
    mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);

    auto cam = std::make_shared<Camera>(CameraType::Ortho);
    cam->updateLocation({0, 0, 2});
    cam->updateLookupTarget({0, 0, -1});
    mdd.camera_shared = cam;
    mdd.viewPortDimensions =  size_;
    mdd.setViewport = true;
    mdd.viewport = {origin_.x,  origin_.y, size_.x, 32};
    mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", glm::vec2{size_.x, 32}}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.04, 0.04, 0.04, 1}}};
    mdd.color = {0.04, 0.04,0.04, 1};
    mdd.location = {2, 2, -1.8};
    mdd.scale = {size_.x - 4 , 30, 1};
    mdd.debugInfo = "menu_bar";
    Renderer::drawWidgetMeshDeferred(mdd, this);

    auto hboxLayout = std::make_shared<HBoxLayout>();
    hboxLayout->setMarginHorizontal(10);
    hboxLayout->apply(this);

    // Next we draw all top level Menus
    for (auto& menu : menus_) {
        menu->draw();
    }
}

MessageHandleResult MenuBar::onMessage(const UIMessage &message) {
    return Widget::onMessage(message);
}
