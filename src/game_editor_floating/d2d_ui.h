//
// Created by mgrus on 15.04.2025.
//

#ifndef D2DUI_H
#define D2DUI_H

#include <d2d1.h>

struct d2d_Widget {
    D2D1_RECT_F bounds;
    int z;
    bool isHovered_ = false;
    bool isPressed_ = false;
    bool isActive_ = false;

    virtual void draw() = 0;

    virtual bool hitTest(POINT pt) const {
        return pt.x >= bounds.left && pt.x <= bounds.right
            && pt.y >= bounds.top && pt.y <= bounds.bottom;
    }
    virtual void onClick() {}

};

#endif //D2DUI_H
