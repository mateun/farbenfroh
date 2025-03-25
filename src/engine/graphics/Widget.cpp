//
// Created by mgrus on 24.03.2025.
//

#include "Widget.h"
#include "Application.h"

class Application;

extern std::shared_ptr<Application> getApplication();

void Widget::resize(int w, int h) {
    this->width = w;
    this->height = h;

}

void Widget::setOrigin(int x, int y) {
    origin_x = x;
    origin_y = y;
}

glm::vec2 Layout::calculateChildSize(const Widget *parent, const Widget *child) {

    return {100, 100};

}

glm::vec2 Layout::calculateChildOrigin(const Widget *parent, const Widget *child) {
    return {10, 10};
}

Container::Container(std::unique_ptr<Layout> layout): layout_(std::move(layout)) {
}

/**
 * As a container which holds child widgets, we follow this rule:
 * - Only the parent is allowed to set the size and origin of the child.
 * - The parent sets the viewport so the child always draws in a neutral and normalized way,
 *   and does not know where exactly it is currently located in terms of the parent space.
 * - Therefore the drawing of the child widget always takes place in child local space.
 */
void Container::draw() {
    for (auto c : children) {
        glm::vec2 size = layout_->calculateChildSize(this, c.get());
        glm::vec2 origin = layout_->calculateChildOrigin(this, c.get());
        c->resize(size.x, size.y);
        c->setOrigin(origin.x, origin.y);
        // TODO is it better to have all this abstracted by the Camera? (projection and viewport)?
        getApplication()->getRenderBackend()->getOrthoCameraForViewport(origin_x, origin_y, size.x, size.y);

        // Now the child can draw itself and all coordinates are based from local 0,0.
        c->draw();

    }

}

void EmptyContainer::draw() {


}
