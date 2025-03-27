//
// Created by mgrus on 27.03.2025.
//

#ifndef TRUETYPETEXTRENDERER_H
#define TRUETYPETEXTRENDERER_H
#include <memory>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <engine/graphics/stb_truetype.h>
#include <engine/graphics/Bitmap.h>
#include <engine/graphics/Texture.h>

#include "Mesh.h"


class TrueTypeTextRenderer {
  public:
    TrueTypeTextRenderer(const std::string& fontFileName);

    // Will return a mesh which represents all quads for the given text.
    // Each quad has a dedicated position following the flow of the text
    // and the correct uvs into the atlas texture.
    // To render the actual text you need to render this mesh
    // and provide the atlasTexture (see getFontAtlas()).
    std::shared_ptr<Mesh> renderText(const std::string& text);
    std::shared_ptr<Texture> getFontAtlas();

private:
    std::vector<stbtt_bakedchar> bakedChars;
    int atlas_bitmap_width_ = 512;
    int atlas_bitmap_height_ = 512;
    GLuint vao;
    GLuint vboPos;
    GLuint vboUVs;
    GLuint vboIndices;
    std::unique_ptr<Bitmap> atlas_bitmap_;
    std::shared_ptr<Texture> atlas_texture_;

};



#endif //TRUETYPETEXTRENDERER_H
