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
#include "TrueTypeFont.h"


class TrueTypeTextRenderer {
  public:
    TrueTypeTextRenderer(const std::shared_ptr<TrueTypeFont>& font);

    // Will return a mesh which represents all quads for the given text.
    // Each quad has a dedicated position following the flow of the text
    // and the correct uvs into the atlas texture.
    // To render the actual text you need to render this mesh
    // and provide the atlasTexture (see getFontAtlas()).
    std::shared_ptr<Mesh> renderText(const std::string& text, glm::vec2* textDimensions);
    std::shared_ptr<Texture> getFontAtlas();

private:

    GLuint vao;
    GLuint vboPos;
    GLuint vboUVs;
    GLuint vboIndices;
    const std::shared_ptr<TrueTypeFont> &font_;
};



#endif //TRUETYPETEXTRENDERER_H
