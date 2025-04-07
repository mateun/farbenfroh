//
// Created by mgrus on 27.03.2025.
//

#include <engine/graphics/TrueTypeFont.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <engine/graphics/stb_truetype.h>
#include <cmath>
#include <memory>
#include <stdexcept>

#include "Bitmap.h"

TrueTypeFont::TrueTypeFont(const std::string &pathToTTF, float fontSize) {
    // Read font file
    FILE *fp = fopen(pathToTTF.c_str(), "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open TTF file.\n");
        throw std::runtime_error("Failed to open TTF file.");
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    unsigned char *ttf_buffer = new unsigned char[size];
    fread(ttf_buffer, 1, size, fp);
    fclose(fp);

    // Define the bitmap dimensions and create a buffer
    atlas_bitmap_ = std::make_unique<Bitmap>(atlas_bitmap_width_, atlas_bitmap_height_, 1);

    // Retrieve font measurements
    stbtt_fontinfo info;
    stbtt_InitFont(&info, ttf_buffer, 0);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    // Then scale them to your desired font size:
    float scale = stbtt_ScaleForPixelHeight(&info, fontSize);
    scaled_ascent_  = ascent  * scale;  // typically a positive number
    scaled_descent_ = descent * scale;  // typically negative
    scaled_line_gap_ = lineGap * scale;

    line_height_ = (scaled_ascent_ - scaled_descent_) + scaled_line_gap_;

    // Allocate an array to hold glyph data for the ASCII range 32..127
    bakedChars.resize(96); // 96 glyphs starting at ASCII 32

    //uint8_t* bitmap = new uint8_t[atlas_bitmap_width_ * atlas_bitmap_height_];



    // Bake the font bitmap for a given font size (e.g., 32 pixels)
    int result = stbtt_BakeFontBitmap(ttf_buffer, 0, fontSize,
                                      atlas_bitmap_->pixels, atlas_bitmap_width_, atlas_bitmap_height_,
                                      32, 96, bakedChars.data());

    if (result <= 0) {
        fprintf(stderr, "Failed to bake font bitmap.\n");
        delete[] ttf_buffer;
        throw std::runtime_error("Failed to bake font bitmap.");
    }

    TextureCreationData td;
    td.internalFormat = GL_R8;
    td.format = GL_RED;
    td.magFilterType = GL_NEAREST;
    td.minFilterType = GL_NEAREST;
    atlas_texture_ = Texture::createTextureFromBitmap(atlas_bitmap_.get(), td);
}

BakedQuadData TrueTypeFont::getBakedQuad(char c, float* penX, float* penY) {
    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(bakedChars.data(), atlas_bitmap_width_, atlas_bitmap_height_, c - 32, penX, penY, &q, 0);

    float x0 = std::floor(q.x0 + 0.5f);
    float y0 = std::floor(q.y0 + 0.5f);
    float x1 = std::floor(q.x1 + 0.5f);
    float y1 = std::floor(q.y1 + 0.5f);

    // float top = -y0;
    // float bottom = -y1;

    BakedQuadData qd;
    qd.x0 = x0;
    qd.y0 = -y0;
    qd.x1 = x1;
    qd.y1 = -y1;
    qd.s0 = q.s0;
    qd.t0 = q.t0;
    qd.s1 = q.s1;
    qd.t1 = q.t1;

    return qd;

}

std::shared_ptr<Texture> TrueTypeFont::getAtlas() {
    return atlas_texture_;
}

float TrueTypeFont::getMaxDescent() {
    return scaled_descent_;
}

float TrueTypeFont::getLineHeight() {
    return line_height_;
}
