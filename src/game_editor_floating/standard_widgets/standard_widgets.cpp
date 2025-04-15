//
// Created by mgrus on 15.04.2025.
//
#include "standard_widgets.h"
#include "../paint_2d.h"

void d2d_CloseButtonWidget::draw() {
    glm::vec4 color = {200, 200, 200, 255};
    //if (isHovered_) color = {250, 20, 20, 255};

    paint2d_draw_custom([this](ID2D1RenderTarget* render_target, ID2D1SolidColorBrush* brush) {
                render_target->FillRectangle(bounds, brush);
              }, {0x0a, 0x0a, 0x0a, 255});

    if (isHovered_) {
        paint2d_draw_custom([this](ID2D1RenderTarget* render_target, ID2D1SolidColorBrush* brush) {
                    render_target->FillRectangle(bounds, brush);
                  }, {250, 20, 20, 255});

    }
    paint2d_draw_custom([this](ID2D1RenderTarget* render_target, ID2D1SolidColorBrush* brush) {
                      D2D_POINT_2F p0 = {bounds.left + 8.0f, bounds.top + 8.0f};
                      D2D_POINT_2F p1 = { bounds.right - 8.0f, bounds.top + 24.0f};
                      D2D_POINT_2F p2 = {bounds.left + 8.0f, bounds.top + 24.0f};
                      D2D_POINT_2F p3 = {bounds.right - 8.0f, bounds.top + 8.0f};

                      render_target->DrawLine(p0, p1, brush, 2);
                      render_target->DrawLine(p2, p3, brush, 2);
                  }, color);




}

void d2d_CloseButtonWidget::onClick() {
    PostQuitMessage(0);
}
