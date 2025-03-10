//
// Created by mgrus on 07.06.2024.
//

#ifndef SIMPLE_KING_DEFAULT_GAME_H
#define SIMPLE_KING_DEFAULT_GAME_H
#include <map>
#include "../../graphics.h"
#include "FolderAssetLoader.h"

class Mesh;
class DefaultGame;

/**
* Every game is only taking place within one level at any time.
* A game may have different levels to which it moves back and forth.
*
*/
class GameLevel {

public:
    GameLevel(DefaultGame* game, const std::string& name = "");
    virtual void update() = 0;
    virtual void render() = 0;
    virtual void init() = 0;
    std::string name();



protected:
    DefaultGame* game = nullptr;
    std::string _name = "";


};


class DefaultGame {

public:
    virtual ~DefaultGame() = default;

    Shader * getDefaultStaticMeshShader();
    Shader* getDefaultSkinnedMeshShader();

    DefaultGame();

    virtual void update();

    virtual void init();

    virtual void render();

    virtual float getFrametimeInSeconds();

    virtual bool shouldStillRun();

    virtual bool useGameLevels();

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

    void registerGameLevel(GameLevel *level);

    // Switches the current level to the new one
    void switchLevel(const std::string& name);

    GameLevel *currentLevel();

protected:
    HWND hwnd;
    int64_t performanceFrequency = 0;
    Shader* baseStaticMeshShader = nullptr;

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
    std::map<std::string, GameLevel*> levels;
    bool useLevels = false;
    GameLevel* _currentLevel;



};


#endif //SIMPLE_KING_DEFAULT_GAME_H
