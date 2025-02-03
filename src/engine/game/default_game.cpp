//
// Created by mgrus on 07.06.2024.
//

#include "default_game.h"
#include <filesystem>
#include <string>
#include <iostream>

Camera* DefaultGame::getGameplayCamera() {
    if (!_gameplayCamera) {
        _gameplayCamera = new Camera();
        _gameplayCamera->location = {0, 5, 5};
        _gameplayCamera->lookAtTarget = {0, 0, -5};
        _gameplayCamera->type = CameraType::Perspective;
    }

    return _gameplayCamera;
}

void DefaultGame::renderFPS() {

    if (!fpsFont) {
        fpsFont = new FBFont("../assets/font.bmp");
    }



    lightingOff();
    bindCamera(getUICamera());
    char buf[175];
    sprintf_s(buf, 160, "FT:%6.1fmcs (%d) %4d/%4d",
              ftMicrosAvg, lastAvgFPS, mouse_x, mouse_y);
    flipUvs(false);
    fpsFont->renderText(buf, {2, -16, -1});


}

Camera *DefaultGame::getUICamera() {
    if (!_uiCamera) {
        _uiCamera = new Camera();
        _uiCamera->location = {0, 0, 0};
        _uiCamera->lookAtTarget = {0, 0, -1};
        _uiCamera->type = CameraType::Ortho;
    }

    return _uiCamera;

}

DefaultGame::DefaultGame()  {

}

void DefaultGame::init() {
    performanceFrequency = performance_frequency;

    // Doing the base class initialiations and then calling the derived class:
    _spriteBatch = new gru::SpriteBatch(1000);
    _uiSpriteBatch = new gru::SpriteBatch(100);

    // Auto asset import, only if allowed:
    folderAssetLoader = new FolderAssetLoader();
    if (shouldAutoImportAssets()) {
        for (const auto& assetFolder : getAssetFolder()) {
            folderAssetLoader->load(assetFolder);
        }
    }

    for (int i = 0; i < 4;i++) {
        controllerStates.push_back(XINPUT_STATE {});
        prevControllerStates.push_back(XINPUT_STATE {});
    }

}

gru::SpriteBatch *DefaultGame::getGameplaySpritebatch() {
    return _spriteBatch;
}

Mesh* DefaultGame::getMeshByName(const std::string &name) {
    return folderAssetLoader->getMesh(name);
}

Sound * DefaultGame::getSoundByName(const std::string &name) {
    return folderAssetLoader->getSound(name);
}

Texture* DefaultGame::getTextureByName(const std::string &name) {
    return folderAssetLoader->getTexture(name);

}

Sound * DefaultGame::getCurrentlyPlayingMusic() {
    return currentMusic;
}

bool DefaultGame::shouldAutoImportAssets() {
    return false;
}

std::vector<std::string> DefaultGame::getAssetFolder() {
    return {};
}

bool DefaultGame::shouldStillRun() {
    return true;
}

std::string DefaultGame::getName() {
    return "GenericGame";
}

std::string DefaultGame::getVersion() {
    return "0.0.1";
}

void DefaultGame::render() {
    bindCamera(getGameplayCamera());
    lightingOn();

    scale({1, 1,1});
    foregroundColor({0.3, 0.6, 0.2, .1});
    location(glm::vec3{0, 0, 0});
    gridLines(100);
    drawGrid();
}

Camera *DefaultGame::getShadowMapCamera() {
    if (!_shadowMapCamera) {
        _shadowMapCamera = new Camera();
        glm::vec3 shadowCamOffsetToNormalCam = {12, -1, 1};
        auto shadowCamLoc = getGameplayCamera()->location + shadowCamOffsetToNormalCam;
        _shadowMapCamera->location =  {shadowCamLoc.x, shadowCamLoc.y, shadowCamLoc.z};
        _shadowMapCamera->lookAtTarget = {-2, 0, -2.99};
        _shadowMapCamera->type = CameraType::OrthoGameplay;
    }

    return _shadowMapCamera;
}

void DefaultGame::setCurrentMusic(Sound *music) {
    this->currentMusic = music;
}

void DefaultGame::stopGame() {
    // noop
}



