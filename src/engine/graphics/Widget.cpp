//
// Created by mgrus on 24.03.2025.
//

#define NOMINMAX
#include "Widget.h"

#include <limits>
#include <ostream>

#include "Application.h"
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/ui/MessageHandleResult.h>

#include "Geometry.h"
#include "ui/MenuBar.h"


class Application;

extern std::shared_ptr<Application> getApplication();


void Widget::setOrigin(glm::vec2 orig) {
    origin_ = orig;
}

glm::vec2 Widget::origin() const {
    return origin_;
}

void Widget::setSize(glm::vec2 size) {
    size_ = size;

}

std::string Widget::getId() const {
    return id_;
}

void Widget::setVisible(bool cond) {
    visible_ = cond;
}

bool Widget::isVisible() const {
    return visible_;
}



glm::vec2 Widget::size() const {
    return size_;
}



Widget::Widget() {
    quadMesh_ = gru::Geometry::createQuadMesh(PlanePivot::bottomleft);
}


/**
 * This is the default message handling implementation - it tries
 * to be as useful as possible, while leaving it open to subclasses to implement their own behavior.
 * We check if we have children - if yes, we send the event to every child until we find one that declared itself
 * as the handler of this event. Then we are happy.
 * If no widget wanted to handle the event, it is also fine, then nothing happens!
 *
 * @param message The incoming message we may or may not "handle".
 */
MessageHandleResult Widget::onMessage(const UIMessage &message) {

    MessageHandleResult handleResult = {};
    if (menu_bar_) {
        handleResult = menu_bar_->onMessage(message);
    }

    // We return early if the menuBar already handled our message, so it is no longer valid
    // and we avoid double hits etc.
    if (handleResult.wasHandled) {
        return handleResult;
    }

    for (auto& c : children_) {
        auto result = c->onMessage(message);
        if (result.wasHandled) return result;
    }

    MessageHandleResult result;
    result.wasHandled = false;
    return result;

}

void Widget::draw(float depth) {
    assert(getApplication()->getRenderBackend() != nullptr);


    // We differentiate two main cases:
    // A. we have a layout
    // B. we don't
    // If we do have a layout, we delegate the sizing/positioning to the layout.
    // If we are lacking a layout, the widget must be placed manually, we just use the origin/size the widget has.
    // It must then have been set somewhere else by the owner of the widget.
    if (layout_) {
        layout_->apply(this);
        if (hasMenuBar()) {
            menu_bar_->setOrigin({origin_.x, size_.y - 32});
            menu_bar_->setSize({size_.x, 32});
            menu_bar_->draw(depth);
        }
        for (auto c : children_) {
            getApplication()->getRenderBackend()->setViewport(c->origin_.x, c->origin_.y,  c->size_.x, c->size_.y);
            c->draw(depth);

        }
    } else {
        if (hasMenuBar()) {
            menu_bar_->setOrigin({origin_.x, size_.y - 32});
            menu_bar_->setSize({size_.x, 32});
            menu_bar_->draw(depth);
        }
        for (auto c : children_) {
            getApplication()->getRenderBackend()->setViewport(c->origin_.x, c->origin_.y,  c->size_.x, c->size_.y);
            c->draw(depth);

        }

    }


}

void Widget::addChild(std::shared_ptr<Widget> child) {
    children_.push_back(child);
}

void Widget::setMenuBar(std::shared_ptr<MenuBar> menu_bar) {
    menu_bar_ = menu_bar;
    getApplication()->getCentralSubMenuManager()->registerMenuBar(menu_bar_);
}

std::vector<std::shared_ptr<Widget>> Widget::children() const{
    return children_;
}

glm::vec2 Widget::getPreferredSize() {
    float minX = std::numeric_limits<float>::max();
    auto maxX = std::numeric_limits<float>::min();
    auto minY = std::numeric_limits<float>::max();
    auto maxY = std::numeric_limits<float>::min();

    for (auto c : children_) {
        auto ps = c->getPreferredSize();
        if (ps.x < minX) minX = ps.x;
        if (ps.x > maxX) maxX = ps.x;
        if (ps.y < minY) minY = ps.y;
        if (ps.y > maxY) maxY = ps.y;
    }

    return {maxX, maxY};
}

glm::vec2 Widget::getMinSize() {
    return getPreferredSize();
}

glm::vec2 Widget::getMaxSize() {
    return {getApplication()->scaled_width(), getApplication()->scaled_height()};
}

void Widget::setLayout(std::shared_ptr<Layout> layout) {
    this->layout_ = layout;
}

bool Widget::hasMenuBar() {
    return menu_bar_ != nullptr;
}

std::shared_ptr<Widget> Widget::getMenuBar() const {
    return menu_bar_;
}

bool Widget::checkMouseOver(int mouse_x, int mouse_y) const {
    if (mouse_x >= origin_.x && mouse_x <= (origin_.x + size_.x) &&
        mouse_y <= (origin_.y + size_.y) && mouse_y >= origin_.y) {
        return true;
    }
    return false;

}

void Widget::setHoverFocus(std::shared_ptr<Widget> prevHolder) {
    // noop
}

void Widget::removeHoverFocus() {
    // noop
}

void Widget::widgetGotHoverFocus(std::shared_ptr<Widget> widget) {
}

void Widget::widgetLostHoverFocus(std::shared_ptr<Widget> widget) {
}

bool Widget::checkMouseOver(int mouse_x, int mouse_y, const Widget* widget, bool useOffsets, glm::vec2 originOffset, glm::vec2 sizeOffset) {
    auto size = useOffsets ? widget->size() + sizeOffset : widget->size();
    auto orig = useOffsets ? widget->origin() + originOffset : widget->origin();

    if (mouse_x >= orig.x && mouse_x <= (orig.x +size.x) &&
        mouse_y <= (orig.y + size.y) && mouse_y >= orig.y) {
        return true;
    }
    return false;

}



void Widget::setId(const std::string &id) {
    id_ = id;
}

std::shared_ptr<Camera> Widget::getDefaultUICam() {
    auto cam = std::make_shared<Camera>(CameraType::Ortho);
    cam->updateLocation({0, 0, 10});
    cam->updateLookupTarget({0, 0, -1});
    return cam;
}




