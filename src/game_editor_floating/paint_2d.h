//
// Created by mgrus on 15.04.2025.
//

#ifndef PAINT_2D_H
#define PAINT_2D_H
#include <d2d1.h>
#include <Windows.h>
#include <glm/glm.hpp>

void paint2d_init(HWND hwnd);
void paint2d_draw_filled_rect(glm::vec2 position, glm::vec2 size, glm::vec4 color) ;
void paint2d_draw_text(const std::wstring& text, glm::vec2 position, glm::vec2 size, glm::vec4 color, IDWriteTextFormat* text_format);

#endif //PAINT_2D_H
