//
// Created by mgrus on 24.03.2025.
//

#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <engine/graphics/Camera.h>

#include "Mesh.h"


class MenuBar;
struct MeshDrawData;
class Layout;
class MessageHandleResult;

enum class MessageType {
    KeyUp,
    KeyDown,
    MouseMove,
    MouseUp,
    MouseDown,
    WidgetGainedFocus,
    WidgetLostFocus,
};

struct MouseMoveMessage {
    int x;
    int y;
};

class Widget;
struct FocusMessage {
    std::shared_ptr<Widget> widget;
};

struct UIMessage {
    MessageType type;
    MouseMoveMessage mouseMoveMessage;
    FocusMessage focusMessage;
    uint64_t num = 0;
    std::string sender;
};

// This struct provides additional hinting
// for layouting e.g. whether to expand,
// stay strict etc.
struct LayoutHint {
    bool expandVertically = false;
    bool expandHorizontally = false;

};

/**
* This is the top level ui element.
* Applications contain exactly one top level widget.
* A widget may have children.
* Layouts allow for automatic placement of child widgets.
* Without a layout, the creator/owner of the widget must set origin and size.
* Otherwise the layout takes care of this.
*/
class Widget : public std::enable_shared_from_this<Widget> {

public:
    Widget();
    virtual ~Widget() = default;

    /**
    * Widgets can get messages regarding mouse movement, mouse clicks,
    * keyboard events etc.
    */
    virtual MessageHandleResult onMessage(const UIMessage& message);

    /**
    * This method allows the widget to draw its visual representation.
    * The caller is responsible to have called "setSize" beforehand so the widget knows how large it is.
    *
    * Please note that the widget normally does NOT immediately draw itself, but provides a list of
    * MeshDrawData objects which describe what the widget wants to draw.
    * This is mainly for performance reasons as it allows the framework to batch the resulting
    * draw commands more efficiently.
    */
    virtual void draw(float depth = -0.5);


    /**
    * Adds a child widget to the hierarchy.
    */
    void addChild(std::shared_ptr<Widget> child);


    /**
    * Widgets can have a MenuBar - this will always be in the
    * top row of the Widget. Every layout must account for the presence
    * or absence of a MenuBar in terms of its positioning.
    */
    void setMenuBar(std::shared_ptr<MenuBar> menuBar);

    /**
    * This is normally only called by a parent widget or layout.
    * Only they can know where this widget should be placed.
    */
    void setOrigin(glm::vec2 origin);

    /**
    * Retrieve the origin of this widget in parent space.
    */
    glm::vec2 origin() const;

    /**
    * This is normally only called by a parent or layout.
    * Only items which are above this widget can know where it
    * should be placed and how large it should be.
    */
    void setSize(glm::vec2 size);

    std::string getId() const;


    /**
    * Retrieve the size of this widget (width, height).
    */
    glm::vec2 size() const;

    /**
    * Get the list of all children.
    */
    std::vector<std::shared_ptr<Widget>> children() const;

    /**
    * This allows the widget to communicate the dim
    */
    virtual glm::vec2 getPreferredSize();

    /**
    * Retrieve the minimum size this widget is allowed to have.
    * The parent layout is not allowed to crop below this.
    */
    virtual glm::vec2 getMinSize();

    /**
    * Retrieve the largest extent this widget ever wants to have.
    */
    virtual glm::vec2 getMaxSize();

    void setLayout(std::shared_ptr<Layout> layout);

    bool hasMenuBar();

    std::shared_ptr<Widget> getMenuBar() const;

    bool checkMouseOver(int mouseX, int mouseY) const;

    // Indicates to the widget it was currently the highest z-value widget where the mouse was hovering over the last
    // frame.
    virtual void setHoverFocus(std::shared_ptr<Widget> prevFocusHolder);

    // Called if this widget no longer has the hover focus.
    virtual void removeHoverFocus();

    // These get called if any widget got or lost focus.
    // This might be important if you need to change internal state based on another type or specific
    // widgets focus state.
    virtual void widgetGotHoverFocus(std::shared_ptr<Widget> widget);
    virtual void widgetLostHoverFocus(std::shared_ptr<Widget> widget);

    void setLayoutHint(LayoutHint hint);
    LayoutHint getLayoutHint();

    static bool checkMouseOver(int mouseX, int mouseY, const Widget* widget, bool useOffsets = false,
                               glm::vec2 originOffset = {0, 0}, glm::vec2 sizeOffset = {0, 0});

    // Returns a standard ortho camera.
    static std::shared_ptr<Camera> getDefaultUICam();

    void setId(const std::string& id);

    void setZValue(int zValue);
    float getZValue() const;

    void setVisible(bool cond);
    bool isVisible() const;

    // Allows to set a background color of a widget, e.g. for debugging
    // or just for aesthetics.
    void setBgColor(glm::vec4 color);
    glm::vec4 getBgColor() const;

    std::weak_ptr<Widget> parent();



protected:

    std::weak_ptr<Widget> parent_;

    // This is an optional identifier, mainly usable for debugging so we know
    // which widget we are seeing currently.
    std::string id_ = "";

    // This is the depth ( or z...) value of the widget.
    float z_value_ = 0.f;

    // This is the location in global space.
    // For the widget drawing itself, it is advised to set its viewport to
    // (0, 0, size.x, size.y)
    glm::vec2 global_origin_ = {};

    // The width and height of this widget.
    // This is mostly not set by the widget itself, but by some parent or
    // layout in the hierarchy above it.
    // The reason is the sizing normally only makes sense when knowing all the participating
    // widgets for a certain area. The widget itself might not be able to position and size itself correctly
    // in relation to the other widgets.
    glm::vec2 global_size_ = {};

    std::vector<std::shared_ptr<Widget>> children_;

    // Our layout which implements automatic positioning and sizing of child widgets.
    std::shared_ptr<Layout> layout_;

    // Most widgets will need a quad mesh to render themselves, so we add one
    // here in the Widget class itself for convenience;
    std::shared_ptr<Mesh> quadMesh_;

    // The top row menu bar.
    std::shared_ptr<MenuBar> menu_bar_;

    bool visible_ = true;

    LayoutHint layout_hint_;
    glm::vec4 bg_color_= {0, 0, 0, 1};
};



inline float Widget::getZValue() const {
    return z_value_;
}

inline void Widget::setZValue(int zValue) {
    z_value_ = zValue;
}





/**
* Manages the placement of the child widgets.
*/
class Layout {
public:

    virtual void apply(Widget* target) = 0;
    float getTopmostY(Widget* target) const;
};

class VBoxLayout : public Layout {
public:
    void apply(Widget* target) override;

    void setMarginHorizontal(int margin);
    void setMarginVertical(int margin);

private:
    int margin_horizontal_ = 4;
    int margin_vertical_ = 4;
};

class HBoxLayout : public Layout {
public:


    void apply(Widget* target) override;

    void setMarginHorizontal(int margin);
    void setMarginVertical(int margin);

private:
    int margin_horizontal_ = 5;
    int margin_vertical_ = 2;
};


class AreaLayout : public Layout {
public:
    AreaLayout(std::shared_ptr<Widget> top, std::shared_ptr<Widget> bottom, std::shared_ptr<Widget> left, std::shared_ptr<Widget> right, std::shared_ptr<Widget> center);
    void apply(Widget* target) override;

    std::shared_ptr<Widget> top_;
    std::shared_ptr<Widget> bottom_;
    std::shared_ptr<Widget> left_;
    std::shared_ptr<Widget> right_;
    std::shared_ptr<Widget> center_;
};



#endif //WIDGET_H
