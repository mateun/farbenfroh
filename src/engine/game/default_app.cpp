//
// Created by mgrus on 07.06.2024.
//

#include "default_app.h"
#include <filesystem>
#include <string>
#include <iostream>

Camera* DefaultApp::getGameplayCamera() {
    if (!_gameplayCamera) {
        _gameplayCamera = new Camera();
        _gameplayCamera->location = {0, 5, 5};
        _gameplayCamera->lookAtTarget = {0, 0, 0};
        _gameplayCamera->type = CameraType::Perspective;
    }

    return _gameplayCamera;
}

void DefaultApp::renderFPS() {

    if (!fpsFont) {
        fpsFont = new FBFont("../assets/font.bmp");
    }



    lightingOff();
    bindCamera(getUICamera());
    char buf[175];
    sprintf_s(buf, 160, "FT:%6.1fmcs (%d) %4d/%4d",
              ftMicrosAvg, lastAvgFPS, mouse_x, mouse_y);
    flipUvs(false);
    fpsFont->renderText(buf, {2, -16, 0.9});


}

Camera *DefaultApp::getUICamera() {
    if (!_uiCamera) {
        _uiCamera = new Camera();
        _uiCamera->location = {0, 0, 1};
        _uiCamera->lookAtTarget = {0, 0, -1};
        _uiCamera->type = CameraType::Ortho;
    }

    return _uiCamera;

}

DefaultApp::DefaultApp()  {

}

void DefaultApp::init() {
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

gru::SpriteBatch *DefaultApp::getGameplaySpritebatch() {
    return _spriteBatch;
}

Mesh* DefaultApp::getMeshByName(const std::string &name) {
    return folderAssetLoader->getMesh(name);
}

Sound * DefaultApp::getSoundByName(const std::string &name) {
    return folderAssetLoader->getSound(name);
}

Texture* DefaultApp::getTextureByName(const std::string &name) {
    return folderAssetLoader->getTexture(name);

}

Sound * DefaultApp::getCurrentlyPlayingMusic() {
    return currentMusic;
}

bool DefaultApp::shouldAutoImportAssets() {
    return false;
}

std::vector<std::string> DefaultApp::getAssetFolder() {
    return {};
}

bool DefaultApp::shouldStillRun() {
    return true;
}

std::string DefaultApp::getName() {
    return "GenericGame";
}

std::string DefaultApp::getVersion() {
    return "0.0.1";
}

void DefaultApp::render() {
    bindCamera(getGameplayCamera());
    lightingOn();

    static auto gd = createGrid(100);
    drawGrid(gd);
}

Camera *DefaultApp::getShadowMapCamera() {
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

void DefaultApp::setCurrentMusic(Sound *music) {
    this->currentMusic = music;
}

void DefaultApp::stopGame() {
    // noop
}



