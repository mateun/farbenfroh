//
// Created by mgrus on 02.04.2025.
//

#ifndef FLOATINGWINDOW_H
#define FLOATINGWINDOW_H

#include <complex.h>
#include <engine/graphics/Widget.h>

#include "RoundedRect.h"

class Texture;
class MessageHandleResult;
struct UIMessage;

class FloatingWindow : public Widget {

public:
    FloatingWindow();
    ~FloatingWindow();

    float getCloseAreaLeft();

    float getCloseAreaBottom();

    void draw(float depth) override;

    void close();

    void renderCloseButtonHover(float depth);

    bool mouseOverCloseButton(int mouse_x, int mouse_y);

    MessageHandleResult onMessage(const UIMessage &message) override;
    void setHoverFocus(std::shared_ptr<Widget> prevFocusHolder) override;
    void removeHoverFocus() override;

    glm::vec2 getCloseAreaUnderLayDimensions();

private:
    bool hover_focus_ = false;
    bool mouse_down_ = false;
    glm::vec2 last_mouse_pos_ = glm::vec2(0.0f);
    glm::vec2 offset_from_pivot_ = glm::vec2(0.0f);
    std::shared_ptr<Texture> closing_icon_texture_;
    bool hovering_close_button_ = false;
    std::shared_ptr<Widget> body_widget_;
    std::shared_ptr<RoundedRect> main_window_rect_;

};



#endif //FLOATINGWINDOW_H
