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
#include <engine/graphics/ui/UIMessage.h>

#include "LabelWidget.h"

Menu::Menu(const std::string& text): text_(text) {
    label_ = std::make_shared<LabelWidget>(text, getApplication()->getDefaultMenuFont());
    label_->setId("label_for_menu_" + text);
}

void Menu::addSubMenu(std::shared_ptr<Menu> subMenu) {
    lazyCreateSubMenuPanel();
    sub_menu_panel_->addChild(subMenu);
    children_.push_back(subMenu);
    sub_menus_.push_back(subMenu);
    subMenu->setParent(std::dynamic_pointer_cast<Menu>(shared_from_this()));
}

void Menu::addMenuItem(std::shared_ptr<MenuItem> menuItem) {
    lazyCreateSubMenuPanel();
    sub_menu_panel_->addChild(menuItem);
    children_.push_back(menuItem);
    items_.push_back(menuItem);
}

void Menu::draw(float depth) {

    // We calculate the width of the underlay to be as big as the subpanel-width,
    // this looks better than a smaller underlay.
    float underLayWidth = parent_.expired() ? global_size_.x + 20 : parent_.lock()->size().x - 5;

    float margin_left = 8;

    z_value_ = depth;
    label_->setOrigin(global_origin_ + glm::vec2{margin_left, 2});
    label_->setSize({underLayWidth, global_size_.y});

    if (app_hover_focus_) {
        // Draw a background quad on hovering.
        MeshDrawData mdd;
        mdd.mesh = quadMesh_;
        mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);
        mdd.viewPortDimensions =  global_size_;
        mdd.setViewport = true;

        if (parent_.expired()) {
            mdd.viewport = {global_origin_.x,  global_origin_.y-4, underLayWidth , global_size_.y};
        } else {
            mdd.viewport = {global_origin_.x-4,  global_origin_.y-4, underLayWidth , global_size_.y};
        }


        mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{0.035, 0.143, 0.298, 0.45}}};
        mdd.color = glm::vec4{0.035, 0.143, 0.298, 0.45};
        mdd.scale = {underLayWidth, global_size_.y , 1};
        mdd.location = {0, 0, depth };
        mdd.debugInfo = id_;
        Renderer::drawWidgetMeshDeferred(mdd, this);

    }

    label_->draw(depth + 0.1);

    // Now for rendering the submenu panel.
    // This is drawn if this menu has children, otherwise we are done.
    // If this menu as children, we render the background panel for those children,
    // and the children:
    if (!children_.empty() && sub_menu_open_) {
        MeshDrawData mdd;
        mdd.mesh = quadMesh_;
        mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);
        mdd.viewPortDimensions =  global_size_;
        mdd.setViewport = true;

        // Calculate size of all children:

        float panelHeight = (getApplication()->getDefaultMenuFont()->getLineHeight() + 8) * children_.size();
        int divisor = 8;
        int panelSizeY = static_cast<int>(std::ceil(panelHeight / divisor)) * divisor;

        // As for the origin, we differentiate between belonging to a top level menu
        // or to a submenu.
        if (parent_menu_.expired()) {
            sub_menu_panel_->setOrigin(global_origin_ - glm::vec2{-1, panelSizeY});
        } else {
            // How much do we move the subpanel to the right?
            int xOffset = parent_menu_.expired() ? 5 : parent_menu_.lock()->size().x + 5;
            xOffset = 172;
            sub_menu_panel_->setOrigin(global_origin_ + glm::vec2{xOffset, 2});
        }

        // Prepare the size of the panel based on the current subMenus/Items:
        sub_menu_panel_->setSize({172, panelSizeY});

        for (auto c : children_) {
            c->setVisible(true);
        }
        sub_menu_panel_->draw(depth + 0.3);
    }

    if (!sub_menu_open_) {
        for (auto c : children_) {
            c->setVisible(false);
        }
    }


}

MessageHandleResult Menu::onMessage(const UIMessage &message) {
    switch (message.type) {

        case MessageType::MouseUp: {
            if (app_hover_focus_) {
                if (sub_menu_panel_) {
                    sub_menu_panel_->onMessage(message);
                } else {
                    if (children_.empty()) {
                        for (auto ab : action_callbacks_) {
                            ab(shared_from_this());
                        }
                        app_hover_focus_ = false;
                        return MessageHandleResult {true, "", true};
                    }
                }
            }
            break;
        }
    }

    return MessageHandleResult {true, "", false};
}

glm::vec2 Menu::getPreferredSize() {
    return label_->getPreferredSize();
}

void Menu::setParent(std::weak_ptr<Menu> parent) {
    parent_menu_ = parent;
}

std::weak_ptr<Menu> Menu::parentMenu() {
    return parent_menu_;
}

void Menu::lazyCreateSubMenuPanel() {
    if (!sub_menu_panel_) {
        sub_menu_panel_ = std::make_shared<Widget>();
        sub_menu_panel_->setBgGradient({0.04, 0.04,0.04, 1}, {0.04, 0.04,0.04, 1});
        auto vbox = std::make_shared<VBoxLayout>();
        vbox->setMarginVertical(0);
        vbox->setMarginHorizontal(4);
        sub_menu_panel_->setLayout(vbox);
    }
}


void Menu::closeSubMenuPanel() {
    sub_menu_open_ = false;
}

bool Menu::isWidgetAChild(const std::shared_ptr<Menu> &menu) {
    return std::find(children_.begin(), children_.end(), menu) != children_.end();

}


bool Menu::isAncestorOf(const std::shared_ptr<Menu> &referenceWidget) {
    if (shared_from_this() == referenceWidget->parentMenu().lock()) {
        return true;
    }

    if (!referenceWidget->parentMenu().expired()) {
        return isAncestorOf(referenceWidget->parentMenu().lock());
    }

    // if (!parent_menu_.expired()) {
    //     return parent_menu_.lock()->isAncestorOf(referenceWidget);
    // }

    return false;

}


void Menu::setHoverFocus(std::shared_ptr<Widget> prevHolder) {
    app_hover_focus_ = true;

    if (!children().empty()) {
        sub_menu_open_ = true;
        getApplication()->getCentralSubMenuManager()->registerSubMenuHolder(std::dynamic_pointer_cast<Menu>(shared_from_this()));
    }

}

void Menu::removeHoverFocus() {
    app_hover_focus_ = false;
}