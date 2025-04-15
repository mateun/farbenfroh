//
// Created by mgrus on 15.04.2025.
//

#ifndef PAINT_2D_H
#define PAINT_2D_H
#include <Windows.h>
#include <glm/glm.hpp>

void paint2d_init(HWND hwnd);
void paint2d_draw_filled_rect(glm::vec2 position, glm::vec2 size, glm::vec4 color) ;

#endif //PAINT_2D_H
