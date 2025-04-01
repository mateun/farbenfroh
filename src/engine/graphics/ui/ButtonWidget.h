//
// Created by mgrus on 30.03.2025.
//

#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H
#include <complex.h>
#include <engine/graphics/Mesh.h>
#include <engine/graphics/Widget.h>
#include <engine/graphics/Texture.h>


class ButtonWidget : public Widget {

public:
    ButtonWidget();
    void draw(float depth = -0.5) override;
    MessageHandleResult onMessage(const UIMessage &message) override;
    glm::vec2 getPreferredSize() override;
    void setTexture(std::shared_ptr<Texture> tex);

private:
    bool hover_ = false;
    std::shared_ptr<Texture> texture_;
};



#endif //BUTTONWIDGET_H
