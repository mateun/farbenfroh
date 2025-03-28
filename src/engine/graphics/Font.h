//
// Created by mgrus on 25.03.2025.
//

#ifndef FONT_H
#define FONT_H

#include <memory>
#include <string>
#include <glm\glm.hpp>
#include <engine/graphics/Texture.h>
#include <engine/graphics/Bitmap.h>

#include "MeshDrawData.h"


class FBFont {
public:
    FBFont(const std::string& fileName);
    void renderText(const std::string& text, glm::vec3 position);

    void clearText();

    void addText(const std::string &text, int x, int y);

    void updateTextTexture(int w, int h);

    void updateAndDrawText(const char *text, int screenPosX, int screenPosY, int screenPosZ);

    void drawText(const char *text, int posx, int posy);

    void drawBitmapTile(int posx, int posy, int tileSize, int tilex, int tiley, Bitmap *bitmap, int offsetX = 0,
                        int offsetY = 0);

    void setPixel(int x, int y, int r, int g, int b, int a);

    void drawBitmap(int posx, int posy, Bitmap *bitmap);

private:
    std::shared_ptr<Texture> texture = nullptr;
    std::shared_ptr<Bitmap> bitmap = nullptr;
    std::shared_ptr<Mesh> quadMesh = nullptr;
};


#endif //FONT_H
