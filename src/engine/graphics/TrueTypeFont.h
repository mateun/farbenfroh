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

  private:
    std::vector<stbtt_bakedchar> bakedChars;
    int atlas_bitmap_width_ = 512;
    int atlas_bitmap_height_ = 512;
    std::unique_ptr<Bitmap> atlas_bitmap_;
    std::shared_ptr<Texture> atlas_texture_;
};



#endif //TRUETYPEFONT_H
