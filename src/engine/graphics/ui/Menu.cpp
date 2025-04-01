//
// Created by mgrus on 31.03.2025.
//

#include "Menu.h"

#include <iostream>
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
    subMenu->setParent(shared_from_this());
}

void Menu::addMenuItem(std::shared_ptr<MenuItem> menuItem) {
    lazyCreateSubMenuPanel();
    sub_menu_panel_->addChild(menuItem);
    children_.push_back(menuItem);
    items_.push_back(menuItem);
}

void Menu::draw(float depth) {
    label_->setOrigin(origin_ + glm::vec2{2, 2});
    label_->setSize(size_);
    if (app_hover_focus_) {
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
        mdd.location = {0, 0, depth };
        mdd.debugInfo = id_;
        Renderer::drawWidgetMeshDeferred(mdd, this);

        // Make sure we raise the depth above the hover highlight marker
        label_->draw(depth + 0.1);

        // Turn on the submenu latch here, because we are hovered, so this
        // condition is satisfied:
        sub_menu_open_ = true;
    }
    else {
        // Just draw the label without underlying hover marking:
        label_->draw(depth + 0.1);


    }

    // Now for rendering the submenu panel.
    // This is drawn if this menu has children, otherwise we are done.
    // If this menu as children, we render the background panel for those children,
    // and the children:
    if (!children_.empty() && sub_menu_open_) {
        MeshDrawData mdd;
        mdd.mesh = quadMesh_;
        mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);
        auto cam = std::make_shared<Camera>(CameraType::Ortho);
        cam->updateLocation({0, 0, 2});
        cam->updateLookupTarget({0, 0, -1});
        mdd.camera_shared = cam;
        mdd.viewPortDimensions =  size_;
        mdd.setViewport = true;

        // Calculate size of all children:
        int panelSizeX = 0;
        int panelSizeY = 0;
        for (auto child : children_) {
            panelSizeY += getPreferredSize().y;
        }


        // As for the origin, we differentiate between belonging to a top level menu
        // or to a submenu.
        if (parent_menu_.expired()) {
            sub_menu_panel_->setOrigin(origin_ - glm::vec2{0, panelSizeY});
            mdd.viewport = {origin_.x-1, origin_.y - panelSizeY, size_.x * 4, panelSizeY };
        } else {
            // How much do we move the subpanel to the right?
            int xOffset = 35;
            sub_menu_panel_->setOrigin(origin_ + glm::vec2{size_.x + xOffset, 0});
            mdd.viewport = {origin_.x + size_.x + xOffset, origin_.y, size_.x * 4, panelSizeY };
        }

        // Prepare the size of the panel based on the current subMenus/Items:
        sub_menu_panel_->setSize({80, panelSizeY});

        // Draw the background for the panel, darker than the highlight of the menu itself:
        mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.1, 0.1, 0.1, 0.3}}};
        mdd.color = glm::vec4{.1, 0.1, 0.1, 0.3};

        mdd.scale = {size_.x * 4, panelSizeY, 1};
        mdd.location = {0, 0, depth + 0.2};
        Renderer::drawWidgetMeshDeferred(mdd, this);

        sub_menu_panel_->draw(depth + 0.3);
    }


}

MessageHandleResult Menu::onMessage(const UIMessage &message) {
    switch (message.type) {
        case MessageType::MouseMove: {
            hover_ = false;

            auto hover_menu = checkMouseOver(message.mouseMoveMessage.x, message.mouseMoveMessage.y);
            bool hover_sub_menu_panel = false;
            if (sub_menu_open_ && sub_menu_panel_) {
                hover_sub_menu_panel = checkMouseOver(message.mouseMoveMessage.x, message.mouseMoveMessage.y, sub_menu_panel_.get(), true, {-35, -3}, {86, 3});
            }
            hover_ = hover_menu;
            hover_sub_panel_ = hover_sub_menu_panel;
            // If we are directly over the menu itself, we are done here:
            if (hover_menu) {
                return MessageHandleResult {true, "", true};
            }

            // If we are over the sub-panel, we want to allow the submenus/items to handle messages themselves:
            if (hover_sub_menu_panel) {
                sub_menu_panel_->onMessage(message);
                return MessageHandleResult {true, "", true};

            } else {
                sub_menu_open_ = false;
            }
            break;

        }
        case MessageType::MouseDown: {
            if (hover_) {
                if (sub_menu_panel_) {
                    sub_menu_panel_->onMessage(message);
                } else {
                    if (children_.empty()) {
                        std::cout << "clicked on menu: " << text_ << std::endl;
                        return MessageHandleResult {true, "", true};
                    }
                }
            }
            break;
        }
    }

    return MessageHandleResult {false, "", false};
}

glm::vec2 Menu::getPreferredSize() {
    return label_->getPreferredSize();
}

void Menu::setParent(std::weak_ptr<Menu> parent) {
    parent_menu_ = parent;
}

void Menu::lazyCreateSubMenuPanel() {
    if (!sub_menu_panel_) {
        sub_menu_panel_ = std::make_shared<Widget>();
        auto vbox = std::make_shared<VBoxLayout>();
        sub_menu_panel_->setLayout(vbox);
    }
}

