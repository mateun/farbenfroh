//
// Created by mgrus on 25.03.2025.
//

#ifndef BITMAP_H
#define BITMAP_H
#include <cinttypes>
#include <Windows.h>
#include <string>

struct BITMAP_FILE {
    BITMAPFILEHEADER bmfileHeader;
    BITMAPINFOHEADER bminfoHeader;

};

struct Bitmap {
    Bitmap();
    Bitmap(int w, int h, int bytes_per_pixel);
    Bitmap(const std::string& file);
    int width=-1;
    int height=-1;
    int bytes_per_pixel_ = 4;
    uint8_t* pixels=nullptr;

};



#endif //BITMAP_H
