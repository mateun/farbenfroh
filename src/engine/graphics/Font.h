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


class FBFont {
public:
    FBFont(const std::string& fileName);
    void renderText(const std::string& text, glm::vec3 position);

private:
    std::unique_ptr<Texture> texture = nullptr;
    std::unique_ptr<Bitmap> bitmap = nullptr;
};


#endif //FONT_H
