//
// Created by mgrus on 02.03.2025.
//

#include "TestTerrainGame.h"
#include "GameplayLevel.h"


DefaultGame* getGame() {
    return new ttg::TestTerrainGame();
}

void ttg::TestTerrainGame::init() {
    DefaultGame::init();
    registerGameLevel("gameplay", new GameplayLevel(this));
    switchLevel("gameplay");
}

void ttg::TestTerrainGame::update() {
    DefaultGame::update();
}

void ttg::TestTerrainGame::render() {
    DefaultGame::render();
}

bool ttg::TestTerrainGame::shouldAutoImportAssets() {
    return true;
}

bool ttg::TestTerrainGame::useGameLevels() {
    return true;
}

std::vector<std::string> ttg::TestTerrainGame::getAssetFolder() {
    return {"../games/opengl/test_terrain/assets"};
}
