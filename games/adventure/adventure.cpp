//
// Created by mgrus on 07.06.2024.
//

#include "adventure.h"
#include "graphics.h"

namespace adv {

    Adventure::Adventure() {
    }

    bool Adventure::shouldStillRun() {
        return true;
    }

    void Adventure::update() {
    }

    void Adventure::render() {
        bindCamera(getGameplayCamera());
        bindTexture(_gameState->texTileMap);
        getGameplaySpritebatch()->render();
        renderFPS();

        //Raytracer().render(320, 200);
    }

    void Adventure::initWalls() {
        auto rightWall = new gru::TextureTile({0,0}, {32, 32}, {512, 512});
        auto leftDoor = new gru::TextureTile({0,288}, {32 , 385}, {512, 512});
        auto fullRoom = new gru::TextureTile({0,32}, {512,276}, {512, 512});
        auto player = new gru::TextureTile({192, 0}, {224, 32}, {512, 512} );

        getGameplaySpritebatch()->addSprite({window_width - 512 - 128, window_height/2}, {1200, (680)}, fullRoom, 0.8f);
        getGameplaySpritebatch()->addSprite({80, window_height/2}, {100, 300}, leftDoor, 0.1);
        getGameplaySpritebatch()->addSprite({380, window_height/2}, {80, 80}, player, 0.2);

    }

    void Adventure::init() {
            DefaultGame::init();

            _gameState = new GameState();
            _gameState->gameObjects = new std::vector<GameObject>(1000);
            _gameState->texTileMap = createTextureFromFile("../assets/binding/main_sprites.png", ColorFormat::RGBA);

            initWalls();

            auto saloonMesh = MeshImporter().importStaticMesh("../assets/saloon.glb");
            auto saloonBvh = gru::Bvh(saloonMesh);

            auto tt = new gru::TextureTile({0,0}, {32, 32}, {512, 512});
//            int col = 50;
//            int row = 50;
//            for (auto& go : *_gameState->gameObjects) {
//                if (col > 1100) {
//                    col = 50;
//                    row += 40;
//                }
//                if (row > 600) {
//                    row = 50;
//                }
//                go.screenPos.x = col ;
//                go.screenPos.y = row;
//                go.sizeInPixels = {32, 32};
//                col +=40;
//                getGameplaySpritebatch()->addSprite(go.screenPos,go.sizeInPixels, tt);
//            }

    }

    std::string Adventure::getAssetFolder() {
        return "../assets/adventure";
    }


} // adv

DefaultGame* getGame() {
    return new adv::Adventure();
}
