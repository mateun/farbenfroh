//
// Created by mgrus on 15.04.2025.
//

#ifndef PAINT_2D_H
#define PAINT_2D_H
#include <d2d1.h>
#include <functional>
#include <Windows.h>
#include <glm/glm.hpp>
#include <string>


void paint2d_init(HWND hwnd);
void paint2d_draw_filled_rect(glm::vec2 position, glm::vec2 size, glm::vec4 color) ;
void paint2d_draw_custom(std::function<void(ID2D1RenderTarget*, ID2D1SolidColorBrush*)> callback, glm::vec4 color);
void paint2d_draw_text(const std::wstring& text, glm::vec2 position, glm::vec2 size, glm::vec4 color, IDWriteTextFormat* text_format);
void paint2d_resize(int w, int h);
void paint2d_begin_draw();
void paint2d_end_draw();
void paint2d_clear(glm::vec4 color);

#endif //PAINT_2D_H
