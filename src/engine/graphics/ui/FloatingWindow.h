//
// Created by mgrus on 02.04.2025.
//

#ifndef FLOATINGWINDOW_H
#define FLOATINGWINDOW_H

#include <engine/graphics/Widget.h>

class Texture;
class MessageHandleResult;
struct UIMessage;

class FloatingWindow : public Widget {

public:
    FloatingWindow();
    ~FloatingWindow();
    void draw(float depth) override;

    void close();

    void renderCloseButtonHover(float depth);

    MessageHandleResult onMessage(const UIMessage &message) override;
    void setHoverFocus(std::shared_ptr<Widget> prevFocusHolder) override;
    void removeHoverFocus() override;

private:
    bool hover_focus_ = false;
    bool mouse_down_ = false;
    glm::vec2 last_mouse_pos_ = glm::vec2(0.0f);
    glm::vec2 offset_from_pivot_ = glm::vec2(0.0f);
    std::shared_ptr<Texture> closing_icon_texture_;
    bool hovering_close_button_ = false;
};



#endif //FLOATINGWINDOW_H
