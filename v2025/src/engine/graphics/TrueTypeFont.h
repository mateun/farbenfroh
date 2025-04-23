//
// Created by mgrus on 27.03.2025.
//

#ifndef TRUETYPEFONT_H
#define TRUETYPEFONT_H

#include <memory>
#include <string>
#include <vector>
#include <engine/graphics/stb_truetype.h>

#include <engine/graphics/Bitmap.h>
#include <engine/graphics/Texture.h>

struct BakedQuadData {
  float x0;
  float x1;
  float y0;
  float y1;
  float s0;
  float t0;
  float s1;
  float t1;
};

class TrueTypeFont {
  public:
    TrueTypeFont(const std::string& pathToTTF, float fontSize);
    BakedQuadData getBakedQuad(char c, float* penX, float* penY);
    std::shared_ptr<Texture> getAtlas();

    float getMaxDescent();
    float getLineHeight();

  private:
    std::vector<stbtt_bakedchar> bakedChars;
    int atlas_bitmap_width_ = 512;
    int atlas_bitmap_height_ = 512;
    std::unique_ptr<gru::Bitmap> atlas_bitmap_;
    std::shared_ptr<Texture> atlas_texture_;
    float line_height_ = -1;
    float scaled_ascent_ = 0;
    float scaled_descent_ = 0;
    float scaled_line_gap_ = 0;
};



#endif //TRUETYPEFONT_H
