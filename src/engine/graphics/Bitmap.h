//
// Created by mgrus on 25.03.2025.
//

#ifndef BITMAP_H
#define BITMAP_H
#include <cinttypes>

struct Bitmap {
    int width=-1;
    int height=-1;
    uint8_t* pixels=nullptr;
};


#endif //BITMAP_H
