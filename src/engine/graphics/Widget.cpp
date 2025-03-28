//
// Created by mgrus on 24.03.2025.
//

#include "Widget.h"
#include "Application.h"
#include <engine/graphics/MeshDrawData.h>

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

glm::vec2 Widget::size() const {
    return size_;
}




Widget::Widget() {


}

void Widget::draw() {
    assert(getApplication()->getRenderBackend() != nullptr);


    // We differentiate two main cases:
    // A. we have a layout
    // B. we don't
    // If we do have a layout, we delegate the sizing/positioning to the layout.
    // If we are lacking a layout, the widget must be placed manually, we just use the origin/size the widget has.
    // It must then have been set somewhere else by the owner of the widget.
    if (layout_) {
        layout_->apply(this);
        for (auto c : children_) {
            getApplication()->getRenderBackend()->setViewport(c->origin_.x, c->origin_.y,  c->size_.x, c->size_.y);
            c->draw();

        }
    } else {
        // TODO what todo without layout


    }


}

void Widget::addChild(std::shared_ptr<Widget> child) {
    children_.push_back(child);
}

std::vector<std::shared_ptr<Widget>> Widget::children() const{
    return children_;
}

glm::vec2 Widget::getPreferredSize() {
    return {getApplication()->scaled_width(), getApplication()->scaled_height()};
}

glm::vec2 Widget::getMinSize() {
    return {0, 0};
}

glm::vec2 Widget::getMaxSize() {
    return {getApplication()->scaled_width(), getApplication()->scaled_height()};
}

void Widget::setLayout(std::unique_ptr<Layout> layout) {
    this->layout_ = std::move(layout);
}




