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
        _gameplayCamera->lookAtTarget = {0, 0, 0};
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
    fpsFont->renderText(buf, {2, -16, 0.1});


}

Camera *DefaultGame::getUICamera() {
    if (!_uiCamera) {
        _uiCamera = new Camera();
        _uiCamera->location = {0, 0, 1};
        _uiCamera->lookAtTarget = {0, 0, -1};
        _uiCamera->type = CameraType::Ortho;
    }

    return _uiCamera;

}

GameLevel::GameLevel(DefaultGame *game, const std::string& name) : game(game), _name(name) {

}

std::string GameLevel::name() {
    return _name;
}

Shader * DefaultGame::getDefaultStaticMeshShader() {
    if (!baseStaticMeshShader) {
        baseStaticMeshShader = new Shader();
        baseStaticMeshShader->initFromFiles("../assets/shaders/base_static_mesh.vert", "../assets/shaders/base.frag");
    }
    return baseStaticMeshShader;
}

Shader * DefaultGame::getDefaultSkinnedMeshShader() {
    // TODO implement
    return nullptr;
}

DefaultGame::DefaultGame()  {

}

void DefaultGame::update() {
    try {
        if (useGameLevels()) {
            _currentLevel->update();
        }
    } catch (const std::exception &e) {
        // Option A: Print to console
        std::cerr << "Unhandled update exception: " << e.what() << std::endl;

        // Option B: Show a message box
        MessageBoxA(
            nullptr,
            e.what(),
            "Unhandled update Exception",
            MB_OK | MB_ICONERROR
        );

        exit(11991122); // or some error code
    }
}

void DefaultGame::init() {
    try {
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
    catch (const std::exception& e) {
        // Option A: Print to console
        std::cerr << "Unhandled init exception: " << e.what() << std::endl;

        // Option B: Show a message box
        MessageBoxA(
            nullptr,
            e.what(),
            "Unhandled init Exception",
            MB_OK | MB_ICONERROR
        );

        exit(06660);
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
    return folderAssetLoader->getTexture(name).get();

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

bool DefaultGame::useGameLevels() {
    return false;
}

std::string DefaultGame::getName() {
    return "GenericGame";
}

std::string DefaultGame::getVersion() {
    return "0.0.1";
}

void DefaultGame::render() {
    if (useGameLevels()) {
        _currentLevel->render();
        return;
    }

    // TODO is this default rendering even useful ?!
    bindCamera(getGameplayCamera());
    lightingOn();

    static auto gd = createGrid(100);
    drawGrid(gd);
}

float DefaultGame::getFrametimeInSeconds() {
    return ftSeconds;
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

void DefaultGame::registerGameLevel(GameLevel *level) {
    levels[level->name()] = level;
}

void DefaultGame::switchLevel(const std::string &name) {
    auto oldLevel = _currentLevel;
    _currentLevel = levels[name];
    _currentLevel->init();
}

GameLevel* DefaultGame::currentLevel() {
    return _currentLevel;
}





