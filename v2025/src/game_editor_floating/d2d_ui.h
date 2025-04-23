//
// Created by mgrus on 15.04.2025.
//

#ifndef D2DUI_H
#define D2DUI_H

#include <d2d1.h>
#include <glm/vec2.hpp>

struct d2d_Widget {
    D2D1_RECT_F bounds_;
    int z;
    bool isHovered_ = false;
    bool isPressed_ = false;
    bool isActive_ = false;
    bool debugDrawBounds_ = false;

    virtual void draw() = 0;
    virtual void drawBounds();

    virtual HCURSOR get_cursor() const;
    virtual bool wants_capture_on_mouse_down() const;
    virtual void on_mouse_down(int mx, int my) {}
    virtual void on_mouse_up(int mx, int my) {}
    virtual void on_mouse_move(int mx, int my) {}
    virtual void on_key_down(int key) {}
    virtual void on_key_up(int key) {}

    virtual bool hitTest(POINT pt) const {
        return pt.x >= bounds_.left && pt.x <= bounds_.right
            && pt.y >= bounds_.top && pt.y <= bounds_.bottom;
    }
    virtual void onClick() {}

};



#endif //D2DUI_H
