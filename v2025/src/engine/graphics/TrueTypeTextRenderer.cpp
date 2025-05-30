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

TrueTypeTextRenderer::TrueTypeTextRenderer(const std::shared_ptr<TrueTypeFont>& font): font_(font) {
    // Prepare our quad mesh.
    // The vertex data and uvs will be updated on each text rendering action.
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);


    glGenBuffers(1, &vboUVs);
    glBindBuffer(GL_ARRAY_BUFFER, vboUVs);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &vboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);

}

TextDimensions TrueTypeTextRenderer::calculateTextDimension(const std::string& text) {
    if (text.empty()) {
        TextDimensions dim;
        dim.dimensions = {0, 0};
        dim.baselineOffset = 0;
        return dim;
    }
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    float x = 0, y = 0;
    for (auto c : text) {


        auto q = font_->getBakedQuad(c, &x, &y);

        // Track min/max for bounding box
        minX = std::min(minX, q.x0);
        maxX = std::max(maxX, q.x1);

        if (c == 32) continue; // ignore space for Y, as this is always zero and messes things up.
        minY = std::min(minY, q.y0); // lowest part (descenders)
        minY = std::min(minY, q.y1);

        maxY = std::max(maxY, q.y0); // highest part (ascenders)
        maxY = std::max(maxY, q.y1);

    }
    TextDimensions dim;
    dim.dimensions = {x, maxY - minY};
    dim.baselineOffset = -minY;

    return dim;

}

std::shared_ptr<Mesh> TrueTypeTextRenderer::renderText(const std::string &text, TextDimensions* textDimensions) {
    glBindVertexArray(vao);
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<uint32_t> indices;

    float x = 0.0f, y = 0.0f;
    float minX =  std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float minY =  std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    int charCounter = 0;
    for (auto c : text) {

        // The character index is offset by 32 (first character baked)
        auto q = font_->getBakedQuad(c, &x, &y);
        // Use the quad data (q.x0, q.y0, q.x1, q.y1 for positions and
        // q.s0, q.t0, q.s1, q.t1 for texture coordinates) to render the character.
        // Typically, you'll render two triangles per quad.


        positions.push_back(glm::vec3(q.x0, q.y0, 0));
        positions.push_back(glm::vec3(q.x1, q.y0, 0));
        positions.push_back(glm::vec3(q.x1, q.y1, 0));
        positions.push_back(glm::vec3(q.x0, q.y1, 0));


        uvs.push_back({q.s0, q.t0});
        uvs.push_back({q.s1, q.t0});
        uvs.push_back({q.s1, q.t1});
        uvs.push_back({q.s0, q.t1});

        int offset = charCounter * 4;
        indices.push_back(2 + offset);indices.push_back(1 + offset);indices.push_back(0 + offset);
        indices.push_back(2 + offset);indices.push_back(0 + offset);indices.push_back(3 + offset);
        charCounter++;

        // Track min/max for bounding box
        minX = std::min(minX, q.x0);
        maxX = std::max(maxX, q.x1);

        if (c == 32) continue; // ignore space for Y, as this is always zero and messes things up.
        minY = std::min(minY, q.y0); // lowest part (descenders)
        minY = std::min(minY, q.y1);

        maxY = std::max(maxY, q.y0); // highest part (ascenders)
        maxY = std::max(maxY, q.y1);

    }
    if (textDimensions) {
        textDimensions->dimensions = {maxX - minX, maxY - minY};
        textDimensions->baselineOffset = -minY;
    }

    GL_ERROR_EXIT(77721);
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

