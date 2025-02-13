//
// Created by mgrus on 07.06.2024.
//

#ifndef SIMPLE_KING_DEFAULT_GAME_H
#define SIMPLE_KING_DEFAULT_GAME_H
#include <map>
#include <Xinput.h>
#include "../../graphics.h"
#include "FolderAssetLoader.h"
#include "input.h"


class Mesh;
class DefaultApp {

public:
    virtual ~DefaultApp() = default;

    DefaultApp();

    virtual void update() = 0;

    virtual void init();

    virtual void render();

    virtual bool shouldStillRun();

    virtual bool shouldAutoImportAssets();

    virtual Camera *getGameplayCamera();

    virtual Camera *getUICamera();

    virtual Camera *getShadowMapCamera();

    virtual std::vector<std::string> getAssetFolder();

    // The name of the game, is e.g. displayed as the window title
    virtual std::string getName();

    // The version of this game build
    virtual std::string getVersion();

    void renderFPS();

    gru::SpriteBatch *getGameplaySpritebatch();

    Mesh *getMeshByName(const std::string &name);

    Sound* getSoundByName(const std::string &name);

    Texture *getTextureByName(const std::string &name);

    Sound* getCurrentlyPlayingMusic();
    void setCurrentMusic(Sound *music);

    virtual void stopGame() ;

protected:
    HWND hwnd;
    int64_t performanceFrequency = 0;

private:
    Camera *_gameplayCamera = nullptr;
    Camera *_uiCamera = nullptr;
    Camera *_shadowMapCamera = nullptr;
    FBFont* fpsFont = nullptr;
    Texture *_fpsTexture = nullptr;
    gru::SpriteBatch *_spriteBatch = nullptr;
    gru::SpriteBatch *_uiSpriteBatch = nullptr;
    FolderAssetLoader *folderAssetLoader = nullptr;
    Sound* currentMusic = nullptr;



};


#endif //SIMPLE_KING_DEFAULT_GAME_H
