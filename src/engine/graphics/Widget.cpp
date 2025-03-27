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

std::shared_ptr<Camera> Widget::getCamera() {
    if (!camera_) {
        camera_ = std::make_shared<Camera>(CameraType::Ortho);
        camera_->updateLocation({0, 0, 2});
        camera_->updateLookupTarget({0, 0, -1});

    }

    return camera_;
}

std::shared_ptr<Shader> Widget::getDefaultWidgetShader() {
    if (!default_widget_shader_) {
        default_widget_shader_ = std::make_shared<Shader>();
        default_widget_shader_->initFromFiles("../src/engine/graphics/shaders/textured_mesh.vert",
            "../src/engine/graphics/shaders/textured_mesh.frag");
    }
    return default_widget_shader_;
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
void Container::draw(Camera* camera) {
    for (auto c : children) {
        glm::vec2 size = layout_->calculateChildSize(this, c.get());
        glm::vec2 origin = layout_->calculateChildOrigin(this, c.get());
        c->resize(size.x, size.y);
        c->setOrigin(origin.x, origin.y);
        // TODO is it better to have all this abstracted by the Camera? (projection and viewport)?
        getApplication()->getRenderBackend()->getOrthoCameraForViewport(origin_x, origin_y, size.x, size.y);
        getApplication()->getRenderBackend()->setViewport(origin_x, origin_y, size.x, size.y);

        // Now the child can draw itself and all coordinates are based from local 0,0.
        c->draw(camera);

    }

}

void EmptyContainer::draw(Camera* camera) {

}
