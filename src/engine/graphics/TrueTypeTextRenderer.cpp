//
// Created by mgrus on 27.03.2025.
//

#include "TrueTypeTextRenderer.h"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "ErrorHandling.h"
#include "stb_truetype.h"

TrueTypeTextRenderer::TrueTypeTextRenderer(const std::string& fontFileName) {
    // 1. read font file
    FILE *fp = fopen(fontFileName.c_str(), "rb");
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

    // 2. Define the bitmap dimensions and create a buffer
    atlas_bitmap_ = std::make_unique<Bitmap>(atlas_bitmap_width_, atlas_bitmap_height_, 1);


    // 3. Allocate an array to hold glyph data for the ASCII range 32..127
    bakedChars.resize(96); // 96 glyphs starting at ASCII 32

    //uint8_t* bitmap = new uint8_t[atlas_bitmap_width_ * atlas_bitmap_height_];

    // 4. Bake the font bitmap for a given font size (e.g., 32 pixels)
    float fontSize = 16.0f;
    int result = stbtt_BakeFontBitmap(ttf_buffer, 0, fontSize,
                                      atlas_bitmap_->pixels, atlas_bitmap_width_, atlas_bitmap_height_,
                                      32, 96, bakedChars.data());

    if (result <= 0) {
        fprintf(stderr, "Failed to bake font bitmap.\n");
        delete[] ttf_buffer;
        throw std::runtime_error("Failed to bake font bitmap.");
    }

    atlas_texture_ = Texture::createTextureFromBitmap(atlas_bitmap_.get(), GL_R8, GL_RED);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    //glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);


    glGenBuffers(1, &vboUVs);
    glBindBuffer(GL_ARRAY_BUFFER, vboUVs);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 4 * 100, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &vboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6 * 100, nullptr, GL_DYNAMIC_DRAW);



}

std::shared_ptr<Mesh> TrueTypeTextRenderer::renderText(const std::string &text) {
    glBindVertexArray(vao);
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<uint32_t> indices;

    float x = 0.0f, y = 0.0f; // starting position
    int charCounter = 0;
    for (auto c : text) {
        stbtt_aligned_quad q;
        // The character index is offset by 32 (first character baked)
        stbtt_GetBakedQuad(bakedChars.data(), atlas_bitmap_width_, atlas_bitmap_height_, c - 32, &x, &y, &q, 0);
        // Use the quad data (q.x0, q.y0, q.x1, q.y1 for positions and
        // q.s0, q.t0, q.s1, q.t1 for texture coordinates) to render the character.
        // Typically, you'll render two triangles per quad.
        float top = -q.y0;
        float bottom = -q.y1;
        positions.push_back(glm::vec3(q.x0, top, 0));
        positions.push_back(glm::vec3(q.x1, top, 0));
        positions.push_back(glm::vec3(q.x1, bottom, 0));
        positions.push_back(glm::vec3(q.x0, bottom, 0));

        uvs.push_back({q.s0, q.t0});
        uvs.push_back({q.s1, q.t0});
        uvs.push_back({q.s1, q.t1});
        uvs.push_back({q.s0, q.t1});

        int offset = charCounter * 4;
        indices.push_back(2 + offset);indices.push_back(1 + offset);indices.push_back(0 + offset);
        indices.push_back(2 + offset);indices.push_back(0 + offset);indices.push_back(3 + offset);
        charCounter++;

    }

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * positions.size(), positions.data(), GL_DYNAMIC_DRAW);
    GL_ERROR_EXIT(7772);

    glBindBuffer(GL_ARRAY_BUFFER, vboUVs);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvs.size(), uvs.data(), GL_DYNAMIC_DRAW);
    GL_ERROR_EXIT(7773);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
    GL_ERROR_EXIT(7774);

    auto mesh = std::make_shared<Mesh>();
    mesh->vao = vao;
    mesh->indices = indices;
    mesh->positions = positions;
    mesh->uvs = uvs;
    return mesh;

}

std::shared_ptr<Texture> TrueTypeTextRenderer::getFontAtlas() {
    return atlas_texture_;
}
