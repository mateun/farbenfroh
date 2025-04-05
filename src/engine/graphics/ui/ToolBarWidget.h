//
// Created by mgrus on 30.03.2025.
//

#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H
#include <engine/graphics/Widget.h>
#include <engine/graphics/Mesh.h>


class ToolBarWidget : public Widget {

public:
    ToolBarWidget();
    void draw(float depth) override;
    MessageHandleResult onMessage(const UIMessage &message) override;

    glm::vec2 getPreferredSize() override;

private:
    std::shared_ptr<HBoxLayout> h_box_layout_;
};



#endif //TOOLBARWIDGET_H
