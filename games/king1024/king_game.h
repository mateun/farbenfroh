//
// Created by mgrus on 19.10.2024.
//

#ifndef SIMPLE_KING_KING_GAME_H
#define SIMPLE_KING_KING_GAME_H
#include "../src/engine/game/default_app.h"
#include "../../src/engine/terrain/planet.h"
#include "physics.h"


namespace king {

    enum class EGameState {
        InSplash,
        InMainMenu,
        InStartBase,
        InShip,
        InStarMap,
        InStrategicMode,
        Settings,
        Exit,

    };


    class KingGameState;

    class KingGame : public DefaultGame {
    public:
        void update() override;
        void init() override;
        void render() override;
        bool shouldStillRun() override;
        bool shouldAutoImportAssets() override;
        std::string getAssetFolder() override;
        std::string getName() override;
        void updateCameraMover();
        CameraMover* getCameraMover();
        JPH::PhysicsSystem* getPhysicsSystem();
        JPH::JobSystemThreadPool* getJobSystem();
        JPH::TempAllocator* getTempAllocator();
        FBFont* getButtonFont();
        void switchState(EGameState state, bool destroyCurrentState, bool recreateNewState);
        int getGold();
        int getStone();
        int getWood();

    protected:
        FBFont* buttonFont = nullptr;

    private:
        fireball::Sphere* planet = nullptr;
        CameraMover* cameraMover = nullptr;
        std::vector<glm::vec3> hitLocations;
        Camera* cockpitCamera = nullptr;
        EGameState gameState = EGameState::InShip;

        JPH::PhysicsSystem* physicsSystem;
        JPH::JobSystemThreadPool* jobSystem = nullptr;
        JPH::TempAllocator* tempAllocator = nullptr;

    private:
        KingGameState* currentGameState = nullptr;
        KingGameState* previousGameState = nullptr;

    private:
        void drawTerrainParts();
        void initJolt();
        bool shouldRun = true;

        // Resources
        int gold = 300;
        int wood = 200;
        int stone = 200;

    };

    class KingGameState {
    public:
        KingGameState(KingGame* game);
        ~KingGameState();
        virtual void update() = 0;
        virtual void init() = 0;
        virtual void render()  = 0;

    protected:
        KingGame* kingGame = nullptr;
    };

} // king

#endif //SIMPLE_KING_KING_GAME_H
