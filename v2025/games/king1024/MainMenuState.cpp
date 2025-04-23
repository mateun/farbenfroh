//
// Created by mgrus on 31.10.2024.
//

#include "MainMenuState.h"

void king::MainMenuState::update() {
    ShowCursor(false);

    bool switchToNewState = false;
    bool destroyOldState = true;
    bool recreateNewState = true;
    EGameState newState = EGameState::InMainMenu;

    // Mouse & Keyboard inputs:
    if (buttonStart->mouseOver() && lbuttonUp) {
        newState = EGameState::InStrategicMode;
        switchToNewState = true;
    };

    if (buttonExit->mouseOver() && lbuttonUp) {
        newState = EGameState::Exit;
        switchToNewState = true;
    };

    if (buttonSettings->mouseOver() && lbuttonUp) {
        newState = EGameState::Exit;
        switchToNewState = true;
        destroyOldState = false;
    }




    if (controllerButtonPressed(ControllerButtons::DPAD_DOWN, 0)) {
        if (menuIndex < 1) {
            menuIndex++;
        }
    }

    if (controllerButtonPressed(ControllerButtons::DPAD_UP, 0)) {
        if (menuIndex > 0) {
            menuIndex--;
            printf("now on start\n");
        }
    }

    if (controllerButtonPressed(ControllerButtons::MENU, 0)) {
        newState = EGameState::Settings;
        switchToNewState = true;
        destroyOldState = false;
    }

    if (controllerButtonPressed(ControllerButtons::A_BUTTON, 0)) {
        printf("x button pressed\n");
        if (menuIndex == 0) {
            newState = EGameState::InStrategicMode;
            switchToNewState = true;
        }

        if (menuIndex == 1) {
            newState = EGameState::Exit;
            switchToNewState = true;
        }
    }


    if (switchToNewState) {
        kingGame->switchState(newState, destroyOldState, recreateNewState);
    } else {
        animationPlayer->update();
    }



}

void king::MainMenuState::render() {

    bindCamera(kingGame->getGameplayCamera());
    // Rocks where the king stands on:
    {
        auto importedMesh = kingGame->getMeshByName("rocks_big");
        bindTexture(nullptr);
        bindMesh(importedMesh);
        location({2, 0, 0});
        rotation({0, 0, 0});
        foregroundColor({0.2, 0.2, 0.3, 1});
        scale({1, 1, 1});
        lightingOn();
        drawMesh();

    }

    // Draw a skinned mesh with the king itself:
    {

        auto importedMesh = kingGame->getMeshByName("king_cartoon");
        bindTexture(kingGame->getTextureByName("king_diffuse"));
        bindMesh(importedMesh);
        location({2, 1.45, 0});
        rotation({0, 0, 0});


        scale({.01, .01, .01});
        setSkinnedDraw(true);
        flipUvs(true);
        lightingOn();
        drawMesh();

        setSkinnedDraw(false);

    }
//    shadowOff();
//
    bindCamera(kingGame->getUICamera());
    lightingOff();

    bindTexture(kingGame->getTextureByName("title_logo"));
    scale({1024, 1024, 1});
    flipUvs(true);
    location({320, scaled_height / 2 - (100/dpiScaleFactor), -2});
    drawPlane();


    bindTexture(kingGame->getTextureByName("btnStart"));
    glm::vec4 col = menuIndex == 0 ? glm::vec4 {1, 0, 0, 1} : glm::vec4{1, 1,1, 1};
    tint(col);
    scale({256, 256, 1});
    location({170, 290, -1.6});
    drawPlane();

    bindTexture(kingGame->getTextureByName("btnExit"));
    scale({256, 256, 1});
    col = menuIndex == 1 ? glm::vec4 {1, 0, 0, 1} : glm::vec4{1, 1,1, 1};
    tint(col);
    location({170, 220, -1.4});
    drawPlane();

//    // Draw the menu arrow
    bindTexture(kingGame->getTextureByName("btnArrowRight"));
    scale({256, 256, 1});
    tint({1, 0, 0, 1});
    float arrowY = menuIndex == 0 ? 294 : 216;
    location({130, arrowY, -1.3});
    drawPlane();
    tint({1, 1, 1, 1});


}

void king::MainMenuState::init() {
    printf("init main menu state\n");

    buttonStart = new FBButton("Start", {100, 480, -1}, {120, 48}, kingGame->getButtonFont());
    buttonSettings = new FBButton("Settings", {100, 400, -1}, {120, 48}, kingGame->getButtonFont());
    buttonExit = new FBButton("Exit", {100, 320, -1}, {120, 48}, kingGame->getButtonFont());

    auto meshName = "king_cartoon";
    auto mesh = kingGame->getMeshByName(meshName);
    peasentIdleAnimation = mesh->animations[0];
    animationPlayer = new AnimationPlayer(peasentIdleAnimation, mesh);
    animationPlayer->play(true);
    mouseVisible(false);

}

king::MainMenuState::MainMenuState(king::KingGame *game) : KingGameState(game) {

}
