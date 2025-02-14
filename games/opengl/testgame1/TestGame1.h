//
// Created by mgrus on 13.02.2025.
//

#ifndef TESTGAME1_H
#define TESTGAME1_H
#include "../src/engine/game/default_app.h"


class TestGame1 : public DefaultApp {

    void init() override;
    void update() override;
    void render() override;
    bool shouldStillRun() override;
    bool shouldAutoImportAssets() override;
    std::vector<std::string> getAssetFolder() override;

    Shader * mechShader = nullptr;
    CameraMover * cameraMover = nullptr;
    Scene * scene = nullptr;
    Light * sun = nullptr;
};



#endif //TESTGAME1_H
