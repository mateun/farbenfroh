//
// Created by mgrus on 02.03.2025.
//

#include "TestTerrainGame.h"
#include "GameplayLevel.h"
#include "LevelEditor.h"


DefaultGame* getGame() {
    return new ttg::TestTerrainGame();
}

void ttg::TestTerrainGame::init() {
    DefaultGame::init();
    registerGameLevel(new GameplayLevel(this));
    registerGameLevel(new LevelEditor(this));
    switchLevel("gameplay");
}

void ttg::TestTerrainGame::update() {
    DefaultGame::update();
    if (keyPressed(VK_F7)) {
        if (currentLevel()->name() == "gameplay") {
            switchLevel("levelEditor");
        } else {
            switchLevel("gameplay");
        }

    }
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
