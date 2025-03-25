//
// Created by mgrus on 24.03.2025.
//

#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>


/**
* This is the top level ui element.
* Applications contain exactly one top level widget.
*/
class Widget {

public:
    virtual void draw() = 0;
    void resize(int w, int h);
    void setOrigin(int x, int y);

protected:
    int width = 0;
    int height = 0;
    int origin_x = 0;
    int origin_y = 0;

};

/**
* Manages the placement of the child widgets.
*/
class Layout {
public:
    glm::vec2 calculateChildSize(const Widget* parent, const Widget* child);
    glm::vec2 calculateChildOrigin(const Widget* parent, const Widget* child);

};

class Container : public Widget {
  public:
    Container(std::unique_ptr<Layout> layout);
    void addChild(std::shared_ptr<Widget> child);
    void draw() override;
    void setSize(int width, int height);

private:
    std::vector<std::shared_ptr<Widget>> children;
    std::unique_ptr<Layout> layout_;
};

class EmptyContainer : public Widget {
public:
    void draw() override;
};



#endif //WIDGET_H
