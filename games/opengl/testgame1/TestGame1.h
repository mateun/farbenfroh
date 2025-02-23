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
    Cinematic* cinematic = nullptr;
    CharacterController * characterController = nullptr;
    UpdateSwitcher* updateSwitcher = nullptr;
    AnimationPlayer * idlePlayer = nullptr;
    AnimationPlayer * walkPlayer = nullptr;
    SceneNode * playerNode = nullptr;
    Shader * skinnedShader = nullptr;
    AnimationController * animationController = nullptr;

};



#endif //TESTGAME1_H
