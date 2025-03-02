//
// Created by mgrus on 13.02.2025.
//

#include "TestGame1.h"
#include "MainMenuLevel.h"
#include "GamePlayLevel.h"

DefaultGame* getGame() {
    return new TestGame1();
}

void TestGame1::init() {
    DefaultGame::init();
    // registerGameLevel("engine_splash", new MainMenuLevel());
    // registerGameLevel("game_splash", new MainMenuLevel());
    registerGameLevel("main_menu", new MainMenuLevel(this));
    registerGameLevel("gameplay", new GamePlayLevel(this));
    switchLevel("main_menu");


}

void TestGame1::update() {
    DefaultGame::update();

}

void TestGame1::render() {
    DefaultGame::render();

}

bool TestGame1::shouldStillRun() {
    return true;
}

bool TestGame1::shouldAutoImportAssets() {
    return true;
}

bool TestGame1::useGameLevels() {
    return true;
}

std::vector<std::string> TestGame1::getAssetFolder() {
    return {"../games/opengl/testgame1/assets"};
}
