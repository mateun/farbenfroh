//
// Created by mgrus on 19.10.2024.
//

#include "_game.h"

namespace king {
    void KingGame::init() {
        DefaultGame::init();
        printf("Hello from GameTemplate 'init'");
    }

    std::string KingGame::getAssetFolder() {
        return "../games/_game_template/assets";
    }

    void KingGame::update() {

    }

    void KingGame::render() {
        DefaultGame::render();
        DefaultGame::renderFPS();
    }

    bool KingGame::shouldStillRun() {
        return true;
    }

    std::string KingGame::getName() {
        return "GameTemplate";
    }
} // king


DefaultGame* getGame() {
    return new king::KingGame();
}