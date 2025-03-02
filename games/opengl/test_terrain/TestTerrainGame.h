//
// Created by mgrus on 02.03.2025.
//

#ifndef TESTTERRAINGAME_H
#define TESTTERRAINGAME_H

#include "../src/engine/game/default_game.h"

namespace ttg {
    class GameplayLevel;


    enum class GameState {
        startup,
        main_menu,
        gameplay,
        settings,
    };



class TestTerrainGame : public DefaultGame {

public:
    void init() override;
    void update() override;
    void render() override;
    bool shouldAutoImportAssets() override;
    bool useGameLevels() override;
    std::vector<std::string> getAssetFolder() override;

private:
    GameplayLevel * gamePlayScene = nullptr;
    GameState state = GameState::startup;

};


};



#endif //TESTTERRAINGAME_H
