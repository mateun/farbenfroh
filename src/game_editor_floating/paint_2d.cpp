//
// Created by mgrus on 15.04.2025.
//
#include <cmath>
#include <d2d1.h>
#include <d2d1helper.h>
#include <functional>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "fonts.h"

// Private fileglobals
static ID2D1Factory* d2d_factory;
static ID2D1HwndRenderTarget* d2d_render_target;
static ID2D1SolidColorBrush* shared_brush;

void paint2d_clear(glm::vec4 color) {
    d2d_render_target->Clear({color.r / 255.0f, color.g/ 255.0f, color.b / 255.0f, color.a / 255.0f});
}

void paint2d_begin_draw() {
    d2d_render_target->BeginDraw();

}

void paint2d_end_draw() {
    d2d_render_target->EndDraw();
}

void paint2d_resize(int w, int h) {
    if (d2d_render_target)  // Only if already created
    {
        D2D1_SIZE_U size = D2D1::SizeU(w, h);
        d2d_render_target->Resize(size);
    }
}

void paint2d_init(HWND hwnd) {
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);
    if (FAILED(hr)) {
      exit(3336);
    }

    RECT rc;
    GetWindowRect(hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(
        rc.right - rc.left,
        rc.bottom - rc.top);

    // Create a Direct2D render target.
    hr = d2d_factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, size),
        &d2d_render_target);

    d2d_render_target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    //d2d_render_target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

    hr = d2d_render_target->CreateSolidColorBrush(
               D2D1::ColorF(0xC7FF00),
               &shared_brush
               );


}

void paint2d_draw_filled_rect(glm::vec2 position, glm::vec2 size, glm::vec4 color) {
    shared_brush->SetColor({color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f});
    D2D1_RECT_F rect;
    rect.left = position.x;
    rect.top = position.y;
    rect.right = position.x + size.x;
    rect.bottom = position.y + size.y;

   // d2d_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
    d2d_render_target->FillRectangle(rect, shared_brush);
}

void paint2d_draw_custom(std::function<void(ID2D1RenderTarget*, ID2D1SolidColorBrush*)> callback, glm::vec4 color) {
    shared_brush->SetColor({color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f});

    callback(d2d_render_target, shared_brush);

}

void paint2d_draw_text(const std::wstring& text, glm::vec2 position, glm::vec2 size, glm::vec4 color, IDWriteTextFormat* text_format) {
    shared_brush->SetColor({color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f});

    float pxX = std::floor(position.x + 0.5f);
    float pxY = std::floor(position.y + 0.5f);
    d2d_render_target->DrawText(
           text.c_str(),
           text.size(),
           text_format,
           D2D1::RectF(pxX, pxY, position.x + size.x, position.y + size.y),
           shared_brush
           );

}
