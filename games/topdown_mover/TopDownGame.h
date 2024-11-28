//
// Created by mgrus on 17.11.2024.
//

#ifndef SIMPLE_KING_TOPDOWNGAME_H
#define SIMPLE_KING_TOPDOWNGAME_H

#include "../src/engine/game/default_game.h"
#include <unordered_map>
#include <map>
#include <string>

enum class TopDownGameState {
    MainMenu,
    DisplayStageWave,
    InGame,
    NextStage,
    ReStart,
    Settings,
    GameOver,
    GameWon,
    Editor,
    Exit
};

class BulletPattern {

public:
    BulletPattern(GameObject* parentGameObject, float speed = 1);
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void render() = 0;
    virtual bool collides(AABB aabb);
    std::vector<GameObject*> getBullets();

protected:
    std::vector<GameObject*> bullets;
    GameObject* parentGameObject;
    float speed = 0;
};

class SpreadLinePattern : public BulletPattern {


public:
    SpreadLinePattern(GameObject* parentGameObject, float speed = 2);
    void init() override;
    void update() override;
    void render() override;


};

enum class EnemyType {
    Seeker,
    PatternShooter,
    AimedShooter
};

enum class ArenaType {
    Round,
    Rect,
};

class EnemyDefinition {
public:
    EnemyType type;
    glm::ivec3 initialLocation = glm::vec3(0, 0, 0);
    glm::ivec3 initialVelocity = glm::vec3(0, 0, 0);


};

class Wave {
    public:
    Wave(const std::string& id);
    std::string id;
    // When does it appear?
    float waveTime = 0;
    std::vector<EnemyDefinition*> enemyDefinitions;

};

class Stage {
    public:
    Stage(const std::string& id);
    std::string id;
    void addWave(Wave* wave);
    std::vector<Wave*> getWaves();

    ArenaType arenaType;

    // The width and height of the arena for this stage.
    glm::ivec2 dimension = glm::vec2(24, 24);

private:
    std::vector<Wave*> waves;
};

class StageSequence {
    public:
    void addStage(Stage* stage);
    std::vector<Stage*> getStages();

private:
    std::vector<Stage*> stages;
};


class Enemy;
class TopDownGame : public DefaultGame {
    void update() override;
    void init() override;
    void render() override;
    bool shouldStillRun() override;
    std::vector<std::string> getAssetFolder() override;
    bool shouldAutoImportAssets() override;

private:
    TopDownGameState gameState = TopDownGameState::InGame;
    TopDownGameState prevGameState = TopDownGameState::InGame;
    GameObject* player = nullptr;
    GameObject *shootGuide = nullptr;
    std::vector<GameObject*> walls;
    std::vector<GameObject*> bullets;
    std::vector<GameObject*> enemyBullets;
    std::vector<Enemy*> seekerEnemies;
    std::vector<Enemy*> shooterEnemies;
    std::vector<Enemy*> sectorPatrolerEnemies;
    FBFont* hudFont = nullptr;

    int mapTilesHorizontal = 25;
    int mapTilesVertical = 25;

    int stage = 0;
    int wave = 0;

    // Player data
    int lives = 3;
    int score = 0;
    bool shootTriggerPressed = false;
    glm::vec3 playerStartPosition = {0, 0, mapTilesVertical/2 - 4};

    bool playerHitThisFrame = false;
    float playerHitTimer = 0;
    float playerMaxHitTime = 0.1;
    bool playerHitByBulletThisFrame = false;
    float playerHitByBulletTimer = 0;
    float playerHitByBulletMaxHitTime = 0.1;

    // Main menu data
    int8_t mainMenuIndex = 0;

    // Settings menu data
    int8_t settingsMenuIndex = 0;

    // GameOver menu data
    int8_t gameOverMenuIndex = 0;

    // GameWon data
    int8_t gameWonMenuIndex = 0;

    // Player can be shortly immune to avoid multiple hits of the same bullet or enemy
    // in too short succession:
    bool playerImmune = false;
    float playerImmuneTimer = 0;
    float playerImmuneMax = 2;

    bool playerBulletImmune = false;
    float playerBulletImmuneTimer = 0;
    float playerBulletImmuneMax = 1;

    AnimationPlayer *playerAnimPlayer = nullptr;
    std::unordered_map<int, bool> enemyCells;


    bool paused = false;
    int stageWaveTimerCounter = 3;
    std::map<std::string, ParticleEffect*> particleEffects;

    void enemyLogicSeekers();
    void enemyLogicShooters();

    void initEnemies();

    void updateInGameOver();
    void updateInGameWon();
    void updateInMainMenu();
    void updateInSettings();
    void updateInEditor();
    void updateInDisplayStageWave();
    void renderInGameOver();
    void renderInGameWon();
    void renderInMainMenu();
    void renderInSettings();
    void renderInEditor();
    void renderInDisplayStageWave();

    // Stage editor
    void renderStageSequenceEditorWindow(bool * p_open);
    void drawPixelIntoEditorCanvas(int x, int y, glm::vec4 color, Bitmap* bitmap);
    void renderWaveGraphicalEditorWindow();


    Stage* currentEditorStage = nullptr;
    Wave* currentEditorWave = nullptr;
    int editorCellSize= 20;
    int editorZoomFactor = 1;
    bool waveGraphicalEditorOpen = false;
    StageSequence* currentEditorstageSequence = nullptr;
    Stage* lastSelectedStageNode = nullptr;
    Wave* lastSelectedWaveNode = nullptr;
    EnemyDefinition* lastSelectedEnemyDefinition = nullptr;

};

class Enemy {

public:
    Enemy(BulletPattern* bulletPattern);

    GameObject* gameObject = nullptr;
    bool wasHitThisFrame = false;
    float hitTimer = 0;
    float maxHitTime = 0.1;
    float shootTimer = 0;
    // This is the interval between pattern shots, i.e.
    // a pattern shoots all the bullets of the pattern at once,
    // but this happens only in this interval.
    float patternShootInterval = 3;

    // This is the interval between single shots.
    float shootInterval = 0.3;

    // TODO remove, probably better inside the patterns.
    float shootAngle = -45;

    float swipeDir = 1;
    float strafeInterval = 5;
    float strafeTimer = 0;
    int strafeDir = 1;
    bool strafing = false;
    int hp = 5;
    BulletPattern * bulletPattern;

    void setHit() {
        hp--;
    }
};



#endif //SIMPLE_KING_TOPDOWNGAME_H
