//
// Created by mgrus on 13.02.2025.
//

#ifndef TESTGAME1_H
#define TESTGAME1_H

#include "../src/engine/game/default_game.h"

class MainMenuLevel;
class GamePlayLevel;

enum class TestGame1State {
    starting_up,
    engine_splash,
    company_splash,
    main_menu,
    game_play,
    game_over,
    settings_main,
    settings_audio,
    settings_video,
    settings_controls,
    settings_gameplay

};


class TestGame1 : public DefaultGame {

public:
    void init() override;
    void update() override;
    void render() override;
    bool shouldStillRun() override;
    bool shouldAutoImportAssets() override;
    bool useGameLevels() override;
    std::vector<std::string> getAssetFolder() override;

private:



    MainMenuLevel * mainMenuScene = nullptr;
    GamePlayLevel * gamePlayScene = nullptr;

    TestGame1State state = TestGame1State::starting_up;
};



#endif //TESTGAME1_H
