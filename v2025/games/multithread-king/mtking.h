//
// Created by mgrus on 13.04.2024.
//

#ifndef SIMPLE_KING_MTKING_H
#define SIMPLE_KING_MTKING_H

#include <map>
#include <string>
#include "FirstPersonController.h"

namespace mtk {

    struct GameObject {
        int id = -1;
        int hp = 100;
        int mana = 50;
        glm::vec2 screenPos;
        glm::vec2 sizeInPixels;
    };


    struct GameState {
        int hp = 100;
        bool shouldRun = true;
        Camera *gamePlayCamera = nullptr;
        Camera *uiCamera = nullptr;
        Camera *shadowMapCamera = nullptr;
        Camera *gunCamera = nullptr;

        Bitmap* font = nullptr;

        Texture* texTree = nullptr;
        Texture* texTileMap = nullptr;
        Texture *fpsTexture = nullptr;
        Texture *baseTexture = nullptr;
        Texture *gunTexture = nullptr;
        Texture* skyBoxTexture = nullptr;

        std::vector<GameObject>* gameObjects = nullptr;


        gru::SpriteBatch* spriteBatch = nullptr;
        gru::SpriteBatch* uiSpriteBatch = nullptr;
        gru::UISystem* uiSystem = nullptr;
        Mesh *plane1Mesh = nullptr;

        std::map<std::string, Mesh *> meshes;

        mtking::Character* character = nullptr;

        mtking::FirstPersonController *fpsController = nullptr;

    };

}


#endif //SIMPLE_KING_MTKING_H
