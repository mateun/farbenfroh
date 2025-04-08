//
// Created by mgrus on 24.03.2025.
//

#define NOMINMAX
#include <engine/graphics/ui/MenuBar.h>
#include "Widget.h"

#include <limits>
#include <ostream>

#include "Application.h"
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/ui/MessageHandleResult.h>
#include <engine/graphics/ui/Action.h>

#include "Geometry.h"
#include "Renderer.h"



class Application;

extern std::shared_ptr<Application> getApplication();


void Widget::setOrigin(glm::vec2 orig) {
    global_origin_ = orig;
}

glm::vec2 Widget::origin() const {
    return global_origin_;
}

void Widget::setSize(glm::vec2 size) {
    global_size_ = size;

}

void Widget::setPreferredSize(glm::vec<2, float> val) {
    preferred_size_ = val;
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

void Widget::setBgColor(glm::vec4 color) {
    use_default_bg_ = false;
    bg_gradient_start_ = color;
    bg_gradient_end_ = color;
}

glm::vec4 Widget::bgColor() const {
    return bg_gradient_start_;
}

void Widget::setBgGradient(glm::vec4 gradientStart, glm::vec4 gradientEnd) {
    use_default_bg_ = false;
    bg_gradient_start_ = gradientStart;
    bg_gradient_end_ = gradientEnd;
}

std::pair<glm::vec4, glm::vec4> Widget::bgGradient() const {
    return { bg_gradient_start_, bg_gradient_end_ };
}

std::weak_ptr<Widget> Widget::parent() {
    return parent_;
}

void Widget::addAction(const std::shared_ptr<Action>& action) {
    actions_.push_back(action);
}

void Widget::addActionCallback(std::function<void(std::shared_ptr<Widget>)> actionCallback) {
    action_callbacks_.push_back(actionCallback);
}


glm::vec2 Widget::size() const {
    return global_size_;
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

void Widget::draw(const float depth) {
    assert(getApplication()->getRenderBackend() != nullptr);

    float backgroundDepth = parent().expired() ? depth + 0.01 : parent().lock()->getZValue() + 0.01;
    // We can render our background in the given color
    {
        MeshDrawData mdd;
        mdd.mesh = quadMesh_;
        mdd.shader = getApplication()->getRenderBackend()->getWidgetDefaultShader(false);
        mdd.viewPortDimensions =  global_size_;
        mdd.setViewport = true;
        mdd.viewport = {global_origin_.x,  global_origin_.y, global_size_.x, global_size_.y};
        glm::vec4 start_color = use_default_bg_ ? glm::vec4{0.01, 0.01, 0.01, 1} : bg_gradient_start_;
        glm::vec4 end_color = use_default_bg_ ? glm::vec4{0.01, 0.01, 0.01, 1} : bg_gradient_end_;
        mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", end_color}};
        //mdd.shaderParameters = {ShaderParameter{"viewPortDimensions", global_size_}, ShaderParameter{"viewPortOrigin", origin()}, ShaderParameter{"gradientTargetColor", glm::vec4{1, 0, 1, 1}}};
        mdd.color = start_color;
        // Place ourselves above the parent depth if we have a parent.
        // Otherwise use the passed in depth override value.
        mdd.location = {0, 0, backgroundDepth };
        mdd.scale = {size().x, size().y, 1.0f};
        mdd.debug_label = "background_" + id_;
        Renderer::drawWidgetMeshDeferred(mdd, this);

    }


    // We differentiate two main cases:
    // A. we have a layout
    // B. we don't
    // If we do have a layout, we delegate the sizing/positioning to the layout.
    // If we are lacking a layout, the widget must be placed manually, we just use the origin/size the widget has.
    // It must then have been set somewhere else by the owner of the widget.
    if (layout_) {
        layout_->apply(this);
        if (hasMenuBar()) {
            menu_bar_->setOrigin({global_origin_.x, global_size_.y - 32});
            menu_bar_->setSize({global_size_.x, 32});
            // For menu bars we assume 0 depth and add a bit here.
            menu_bar_->draw(0.01);
        }
            for (auto c : children_) {
            getApplication()->getRenderBackend()->setViewport(c->global_origin_.x, c->global_origin_.y,  c->global_size_.x, c->global_size_.y);
            c->draw(backgroundDepth + 0.01);

        }
    } else {
        if (hasMenuBar()) {
            menu_bar_->setOrigin({global_origin_.x, global_size_.y - 32});
            menu_bar_->setSize({global_size_.x, 32});
            menu_bar_->draw(0.01);
        }
        for (auto c : children_) {
            getApplication()->getRenderBackend()->setViewport(c->global_origin_.x, c->global_origin_.y,  c->global_size_.x, c->global_size_.y);
            c->draw(backgroundDepth + 0.01);

        }

    }


}

void Widget::addChild(std::shared_ptr<Widget> child) {
    children_.push_back(child);
    child->parent_ = shared_from_this();
}

void Widget::setMenuBar(std::shared_ptr<MenuBar> menu_bar) {
    menu_bar_ = menu_bar;
    getApplication()->getCentralSubMenuManager()->registerMenuBar(menu_bar_);
}

std::vector<std::shared_ptr<Widget>> Widget::children() const{
    return children_;
}

glm::vec2 Widget::getPreferredSize() {

    if (children_.empty()) {
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

        // TODO for now return some default size if we are zero sized currently.
        // As this would make us totally invisible for any kind of layout applied.
        if (maxX <= 0.01 && maxY <= 0.01) {
            return preferred_size_;
        }

        return {maxX, maxY};
    }
    return preferred_size_;

}

void Widget::setLayoutHint(LayoutHint layout_hint) {
    layout_hint_ = layout_hint;
}

LayoutHint Widget::getLayoutHint() {
    return layout_hint_;
}

void Widget::clearChildren() {
    children_.clear();
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
    if (mouse_x >= global_origin_.x && mouse_x <= (global_origin_.x + global_size_.x) &&
        mouse_y <= (global_origin_.y + global_size_.y) && mouse_y >= global_origin_.y) {
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




