//
// Created by mgrus on 15.04.2025.
//
#include "standard_widgets.h"
#include "../paint_2d.h"
#include "../fonts.h"

static HCURSOR arrow = LoadCursor(NULL, IDC_ARROW);
static HCURSOR size_all = LoadCursor(NULL, IDC_SIZEALL);

static d2d_FontCache& font_cache() {
    static d2d_FontCache cache;
    return cache;
}

IDWriteTextFormat * d2d_FontCache::get_or_create(const std::wstring &fontName, float size,
    TextAlignHorizontal horizontal, TextAlignVertical vertical) {
    auto key = std::make_tuple(fontName, size, horizontal, vertical);
    auto it = formats_.find(key);
    if (it != formats_.end()) {
        return it->second;
    }
    IDWriteTextFormat* new_format_;
    createDWriteFont(fontName, &new_format_, size);
    return new_format_;

}

d2d_Label::d2d_Label(const std::wstring &text) {
    text_ = text;
}

void d2d_Label::draw() {
    if (!cached_text_format_) {
        cached_text_format_ = font_cache().get_or_create(font_name_, font_size_, align_horizontal_, align_vertical_);
    }
    const auto& b = bounds_;
    paint2d_draw_text(text_, {b.left, b.top}, {b.right - b.left, b.bottom - b.top}, text_color_, cached_text_format_);

}

d2d_Button::d2d_Button(D2D1_RECT_F bounds) {
    this->bounds_ = bounds;
}

void d2d_Button::draw() {
    const auto&  b = bounds_;
    paint2d_draw_filled_rect({b.left, b.top}, {b.right - b.left, b.bottom - b.top}, {0, 0, 0, 255});
    if (isHovered_) {
        paint2d_draw_filled_rect({bounds_.left, bounds_.top}, {bounds_.right - bounds_.left, bounds_.bottom - bounds_.top}, bg_hover_color_);
    } else {
        paint2d_draw_filled_rect({bounds_.left, bounds_.top}, {bounds_.right - bounds_.left, bounds_.bottom - bounds_.top}, bg_color_);
    }

    if (label_) {
        label_->bounds_ = {bounds_.left + 8, bounds_.top + 8, bounds_.right -8, bounds_.bottom - 8};
        label_->draw();
    }



}

void d2d_Button::onClick() {
    if (click_callback_) {
        click_callback_();
    }
}

void d2d_CloseButtonWidget::draw() {
    glm::vec4 color = {200, 200, 200, 255};

    if (isHovered_) {
        paint2d_draw_custom([this](ID2D1RenderTarget* render_target, ID2D1SolidColorBrush* brush) {
                    render_target->FillRectangle(bounds_, brush);
                  }, {250, 20, 20, 255});

    }
    paint2d_draw_custom([this](ID2D1RenderTarget* render_target, ID2D1SolidColorBrush* brush) {
                      D2D_POINT_2F p0 = {bounds_.left + 8.0f, bounds_.top + 8.0f};
                      D2D_POINT_2F p1 = { bounds_.right - 8.0f, bounds_.top + 24.0f};
                      D2D_POINT_2F p2 = {bounds_.left + 8.0f, bounds_.top + 24.0f};
                      D2D_POINT_2F p3 = {bounds_.right - 8.0f, bounds_.top + 8.0f};

                      render_target->DrawLine(p0, p1, brush, 2);
                      render_target->DrawLine(p2, p3, brush, 2);
                  }, color);




}

void d2d_CloseButtonWidget::onClick() {
    PostQuitMessage(0);
}

d2d_GripWidget::d2d_GripWidget(HWND window_to_drag) {
    hwnd_to_drag_ = window_to_drag;
}

void d2d_GripWidget::draw() {
    const auto&  b = bounds_;
    glm::vec2 center = {b.left + (b.right - b.left)/2, b.top + (b.bottom - b.top)/2};

    if (isHovered_) {
        const auto&  b = bounds_;
        paint2d_draw_filled_rect({b.left, b.top}, {b.right -b.left, b.bottom - b.top}, {60, 60, 60, 70});
    }

    if (orientation_ == GripOrientation::Horizontal) {
            paint2d_draw_custom( [this,b, center](ID2D1RenderTarget* render_target, ID2D1SolidColorBrush* brush) {
                float x = center.x - spacing;
                for (int col = 0; col < 3; col++) {
                    float y = center.y - spacing;
                    for (int i = 0; i < 2; ++i) {
                        D2D1_ELLIPSE ellipse;
                        ellipse.point = {x + col * spacing, y  + (i * spacing)};
                        ellipse.radiusX = dotRadius;
                        ellipse.radiusY = dotRadius;
                        render_target->FillEllipse(ellipse, brush);
                    }
                }

            }, dotColor);

    }
    // Inset line:
    paint2d_draw_filled_rect({center.x - 8, b.bottom-15}, {16, 1}, {90, 90, 90, 200});


    if (debugDrawBounds_) {
        drawBounds();
    }
}

HCURSOR d2d_GripWidget::get_cursor() const {
    if (isHovered_) {
        return size_all;
    }
    return arrow;

}

void d2d_GripWidget::on_mouse_down(int mouse_x, int mouse_y) {
    if (!hwnd_to_drag_) return;

    if (isHovered_) {
        dragging = true;
        POINT pt = { mouse_x, mouse_y };
        ClientToScreen(hwnd_to_drag_, &pt);  // convert to screen coords
        drag_start_mouse_ = {pt.x, pt.y};

        RECT r;
        GetWindowRect(hwnd_to_drag_, &r);
        drag_start_window_ = { r.left, r.top };
        SetCursor(LoadCursor(NULL, IDC_SIZEALL));
    }

}

void d2d_GripWidget::on_mouse_move(int mouse_x, int mouse_y) {
    if (!dragging) return;

    POINT pt = { mouse_x, mouse_y };
    ClientToScreen(hwnd_to_drag_, &pt);  // convert to screen coords

    glm::vec2 delta = glm::vec2{pt.x, pt.y} - drag_start_mouse_;
    int new_x = drag_start_window_.x + (int)delta.x;
    int new_y = drag_start_window_.y + (int)delta.y;
    SetWindowPos(hwnd_to_drag_, nullptr, new_x, new_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void d2d_GripWidget::on_mouse_up(int mouse_x, int mouse_y) {
    dragging = false;
    SetCursor(LoadCursor(NULL, IDC_ARROW));
}

bool d2d_GripWidget::wants_capture_on_mouse_down() const {
    return true;
}

void d2d_Widget::drawBounds() {
    paint2d_draw_custom([this](ID2D1RenderTarget* render_target, ID2D1SolidColorBrush* brush) {
                 render_target->DrawRectangle(bounds_, brush);
               }, {0xff, 0xFF, 0xff, 255});
}

HCURSOR d2d_Widget::get_cursor() const {
    return arrow;
}

bool d2d_Widget::wants_capture_on_mouse_down() const {
    return false;
}
