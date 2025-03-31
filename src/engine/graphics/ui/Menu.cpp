//
// Created by mgrus on 31.03.2025.
//

#include "Menu.h"

#include <engine/graphics/Application.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/ui/MessageHandleResult.h>
#include <engine/graphics/ui/MenuItem.h>

#include "LabelWidget.h"

Menu::Menu(const std::string& text): text_(text) {
    auto font = std::make_shared<TrueTypeFont>("../assets/calibri.ttf", 13);
    label_ = std::make_shared<LabelWidget>(text, font);
}

void Menu::addSubMenu(std::shared_ptr<Menu> subMenu) {
    lazyCreateSubMenuPanel();
    sub_menu_panel_->addChild(subMenu);
    children_.push_back(subMenu);
    sub_menus_.push_back(subMenu);
}

void Menu::addMenuItem(std::shared_ptr<MenuItem> menuItem) {
    lazyCreateSubMenuPanel();
    sub_menu_panel_->addChild(menuItem);
    children_.push_back(menuItem);
    items_.push_back(menuItem);
}

void Menu::draw() {
    label_->setOrigin(origin_ + glm::vec2{2, 2});
    label_->setSize(size_);

    if (hover_) {
        // Draw a background quad on hovering.
        MeshDrawData mdd;
        mdd.mesh = quadMesh_;
        mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);

        // TODO create helper for default ui ortho cam
        auto cam = std::make_shared<Camera>(CameraType::Ortho);
        cam->updateLocation({0, 0, 2});
        cam->updateLookupTarget({0, 0, -1});
        mdd.camera_shared = cam;
        mdd.viewPortDimensions =  size_;
        mdd.setViewport = true;
        mdd.viewport = {origin_.x-1,  origin_.y, size_.x + 7, size_.y + 4};
        mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.2, 0.2, 0.2, 0.3}}};
        mdd.color = glm::vec4{.2, 0.2, 0.2, 0.3};
        mdd.scale = {size_.x + 7, size_.y +4 , 1};
        mdd.location = {0, 0, -1.1};
        mdd.debugInfo = id_;
        Renderer::drawWidgetMeshDeferred(mdd, this);


        // If this menu as children, we render the background panel for those children,
        // and the children:
        if (!children_.empty()) {

            // Calculate size of all children:
            int panelSizeX = 0;
            int panelSizeY = 0;
            for (auto child : children_) {
                panelSizeY += 30;
            }

            // Prepare the size of the panel based on the current subMenus/Items:
            sub_menu_panel_->setOrigin(origin_ - glm::vec2{0, panelSizeY});
            sub_menu_panel_->setSize({80, panelSizeY + 10});

            // Draw the background for the panel
            mdd.viewport = {origin_.x-1, origin_.y - panelSizeY, size_.x * 4, panelSizeY };
            mdd.scale = {size_.x * 4, panelSizeY, 1};
            mdd.location = {0, 0, -.5};
            Renderer::drawWidgetMeshDeferred(mdd, this);

            sub_menu_panel_->draw();
        }


    }

    label_->draw();
}

MessageHandleResult Menu::onMessage(const UIMessage &message) {
    switch (message.type) {
        case MessageType::MouseMove: {
            hover_ = checkMouseOver(message.mouseMoveMessage.x, message.mouseMoveMessage.y);
            if (hover_) {
                return MessageHandleResult {true, "", true};
            }
        }
        case MessageType::MouseDown: {
            break;
        }
    }

    return MessageHandleResult {false, "", false};
}

glm::vec2 Menu::getPreferredSize() {
    return label_->getPreferredSize();
}

void Menu::lazyCreateSubMenuPanel() {
    if (!sub_menu_panel_) {
        sub_menu_panel_ = std::make_shared<Widget>();
        auto vbox = std::make_shared<VBoxLayout>();
        sub_menu_panel_->setLayout(vbox);
    }
}

