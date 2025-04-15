//
// Created by mgrus on 15.04.2025.
//

#ifndef FONTS_H
#define FONTS_H
#include <string>

struct ID2D1Factory;
struct IDWriteTextFormat;

void createDWriteFont(const std::wstring& fontName, IDWriteTextFormat** targetTextFormat, float size);

#endif //FONTS_H
