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
};

struct MouseMoveMessage {
    int x;
    int y;
};

struct UIMessage {
    MessageType type;
    MouseMoveMessage mouseMoveMessage;

};

/**
* This is the top level ui element.
* Applications contain exactly one top level widget.
* A widget may have children.
* Layouts allow for automatic placement of child widgets.
* Without a layout, the creator/owner of the widget must set origin and size.
* Otherwise the layout takes care of this.
*/
class Widget {

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
    virtual void draw();


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

protected:
    // This is the location in parent-space.
    // For the widget itself this is normally not "interesting",
    // as for its own drawing purposes it always assumes a (0,0) origin
    // in its own space.
    // We store the origin here nonetheless so the widget can be asked by its parent
    // or layout manager where it actually is located.
    glm::vec2 origin_ = {};

    // The width and height of this widget.
    // This should never be set by the widget itself, but by some parent or
    // layout in the hierarchy above it.
    // The reason is the layouting only makes sense when knowing all the participating
    // widgets for a certain area, the widget itself could never position and size itself correctly.
    // The dimension is though important for self drawing, as it gives the widget the information how big it actually is.
    glm::vec2 size_ = {};

    std::vector<std::shared_ptr<Widget>> children_;

    // Our layout which implements automatic positioning and sizing of child widgets.
    std::shared_ptr<Layout> layout_;

    // Most widgets will need a quad mesh to render themselves, so we add one
    // here in the Widget class itself for convenience;
    std::shared_ptr<Mesh> quadMesh_;

    // The top row menu bar.
    std::shared_ptr<MenuBar> menu_bar_;
};


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
    int margin_horizontal_ = 2;
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
