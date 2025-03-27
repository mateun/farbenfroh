//
// Created by mgrus on 24.03.2025.
//

#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <engine/graphics/Camera.h>


/**
* This is the top level ui element.
* Applications contain exactly one top level widget.
*/
class Widget {

public:
    virtual void draw(Camera* camera) = 0;
    void resize(int w, int h);
    void setOrigin(int x, int y);
    bool isContainer();

    glm::vec2 origin() const;

protected:
    int width = 0;
    int height = 0;
    int origin_x = 0;
    int origin_y = 0;

    std::shared_ptr<Camera> getCamera();
    std::shared_ptr<Shader> getDefaultWidgetShader();

private:
    std::shared_ptr<Camera> camera_;
    std::shared_ptr<Shader> default_widget_shader_;

};

class Container;

/**
* Manages the placement of the child widgets.
*/
class Layout {
public:
    virtual glm::vec2 calculateChildSize(const Container* parent, const Widget* child);
    virtual glm::vec2 calculateChildOrigin(const Container* parent, const Widget* child);
};

class VBoxLayout : public Layout {
public:
    glm::vec2 calculateChildOrigin(const Container *parent, const Widget *child) override;
    glm::vec2 calculateChildSize(const Container *parent, const Widget *child) override;
};

class Container : public Widget {

  public:
    Container(std::unique_ptr<Layout> layout);
    void addChild(std::shared_ptr<Widget> child);
    void draw(Camera* camera) override;
    void setSize(int width, int height);
    std::vector<std::shared_ptr<Widget>> children() const;

private:
    std::vector<std::shared_ptr<Widget>> children_;
    std::unique_ptr<Layout> layout_;
};

class EmptyContainer : public Widget {
public:
    void draw(Camera* camera) override;
};



#endif //WIDGET_H
