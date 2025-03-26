//
// Created by mgrus on 26.03.2025.
//

#ifndef UIBUTTON_H
#define UIBUTTON_H

#include <glm\glm.hpp>
#include <string>
#include <engine/graphics/SpriteBatch.h>

namespace gru {

struct UIButton {
    glm::vec2 pos;
    glm::vec2 size;
    std::string name;
    int spriteIndex = -1;
    SpriteBatch* spriteBatch = nullptr;

    glm::vec2 savedSize;

    void scaled(float val) {
        savedSize = size;
        spriteBatch->updateSprite(spriteIndex, pos, {size * val});
    }
    void unscaled() {
        size = savedSize;
        spriteBatch->updateSprite(spriteIndex, pos, savedSize);
    }
};

}



#endif //UIBUTTON_H
