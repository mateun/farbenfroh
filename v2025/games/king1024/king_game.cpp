//
// Created by mgrus on 19.10.2024.
//

#include "king_game.h"
#include "InsideShipState.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "physics.h"
#include "InsideShipState.h"
#include "MainMenuState.h"
#include "SettingsState.h"
#include "BuildingGameplay.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/Core.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Constraints/Constraint.h>




namespace king {

    void KingGame::init() {

        // First, prepare our base game.
        // This provides services for the actual game states.
        DefaultGame::init();

        getGameplayCamera()->updateLocation({0, 1.8, 5});
        getGameplayCamera()->updateLookupTarget({0, 1.8, -20});
        cameraMover = new CameraMover(getGameplayCamera());

        initJolt();

        buttonFont = new FBFont("../assets/font.bmp");


        cockpitCamera = new Camera();
        cockpitCamera->type = CameraType::Perspective;
        cockpitCamera->updateLocation({0, 1.5, 1.5});
        cockpitCamera->updateLookupTarget({0,1.5, -2});

        auto rightRotMat = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0,0,-1));
        auto leftRotMat = glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0,0,-1));

        planet = new fireball::Sphere(15, {0, 0, 0});

        switchState(EGameState::InMainMenu, true, true);
    }

    std::string KingGame::getAssetFolder() {
        return "../games/king1024/assets";
    }

    /**
     * Switches the current state to the state symbolized by the incoming name.
     *
     * @param stateName             The new target state.
     * @param destroyCurrentState   This allows the current state to be kept alive,
     *                              e.g. for temporary pause menu states
     */
    void KingGame::switchState(EGameState state, bool destroyCurrentState, bool recreateNewState) {
        if (!recreateNewState) {
            auto newCurrent  = previousGameState;
            if (destroyCurrentState) {
                delete(currentGameState);
                currentGameState = newCurrent;
            }
            return;
        }

        previousGameState = currentGameState;
        if (currentGameState && destroyCurrentState) {
            delete(currentGameState);
            currentGameState = nullptr;
        }

        gameState = state;

        switch (state) {
            case EGameState::Exit:
                shouldRun = false;
                break;
            case EGameState::Settings:
                currentGameState = new SettingsState(this);
                break;
            case EGameState::InShip:
                currentGameState = new InsideShipState(this, getPhysicsSystem()->GetBodyInterface());
                break;
            case EGameState::InStrategicMode:
                currentGameState = new BuildingGameplay(this);
                break;
            case EGameState::InMainMenu:

                    currentGameState = new MainMenuState(this);
                break;
            default: printf("state not found!");
        }

        if (currentGameState && recreateNewState) {
            currentGameState->init();
        }

    }

    // Dispatch to the current states update method.
    void KingGame::update() {

        if (currentGameState) {
            currentGameState->update();
        }
        return;


        cameraMover->update();

        if (lbuttonUp) {
            auto ray = createRayFromCameraAndMouse(getGameplayCamera());
            float intersectionDistance = -1;
            glm::vec3 intersectionPoint;
            auto didHit = rayIntersectsSphere(ray, planet->getRadius(), planet->getWorldLocation(),intersectionDistance, intersectionPoint);
            if (didHit) {
                printf("hit planet at: %f/%f/%f\n", intersectionPoint.x, intersectionPoint.y, intersectionPoint.z);
                hitLocations.push_back(intersectionPoint);
            }
        }

    }

    void KingGame::render() {

        if (currentGameState) {
            currentGameState->render();
        }
        return;

        // Early exit for debugging for now.
        //return;

        static float frameRot = 0;
        float rotSpeed = 0;
        frameRot += ftSeconds * rotSpeed;

        DefaultGame::render();

        //drawTerrainParts();

        // Draw the other planet a bit to the right
        bindTexture(nullptr);
        location(planet->getWorldLocation());
        foregroundColor({0, 0.5, 0, 1});
        rotation({frameRot, frameRot/2, frameRot*1.5});
        lightingOn();
        planet->render();
        rotation({0, 0, 0});

        // Debug draw our hit locations
        foregroundColor({1, 0, 0, 1});
        scale({0.02, 0.02, 0.02});
        bindMesh(getMeshByName("saloon"));
        for (auto hl : hitLocations) {
            location(hl);
            auto locationNormal = (glm::normalize(hl));
            auto quatRot = calculateRotation({0, 1, 0}, locationNormal);
            auto rotationMatrix = glm::toMat4(quatRot);
            rotation(&rotationMatrix);
            drawMesh();
            rotation(nullptr);
        }

        // Draw a saloon as landmark:
        location({40, 0, 0});
        bindMesh(getMeshByName("saloon"));
        rotation({0, 0, 0});
        foregroundColor({0, 0, 1, 1});
        scale({1, 1, 1});
        drawMesh();

        // Draw ourselves in the cockpit always
        // (like an FPS weapon)
        if (false)
        {
            bindCamera(cockpitCamera);
            bindTexture(getTextureByName("cockpit_albedo"));
            flipUvs(true);
            bindMesh(getMeshByName("spaceship_cockpit"));
            scale({1, 1,1});
            location({0, 0, 0});
            rotation({0, 0, 0});
            drawMesh();
            flipUvs(false);
            rotation({0, 0, 0});
        }

        // Crosshair in the middle of the screen
        bindCamera(getUICamera());
        wireframeOn();
        lightingOff();
        foregroundColor({1, 1, 1, 1});
        scale({16, 16, 1});
        location({window_width/2, window_height/2, -2});
        drawPlane();

        wireframeOff();



        DefaultGame::renderFPS();
    }

    bool KingGame::shouldStillRun() {
        return shouldRun;
    }

    std::string KingGame::getName() {
        return "King1024";
    }

    bool KingGame::shouldAutoImportAssets() {
        return true;
    }

    void KingGame::updateCameraMover() {
        cameraMover->update();
    }

    CameraMover *KingGame::getCameraMover() {
        return cameraMover;
    }

    void KingGame::initJolt() {
        using namespace JPH;

        RegisterDefaultAllocator();

        Trace = TraceImpl;
        JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

        // Create a factory
        Factory::sInstance = new JPH::Factory();

        RegisterTypes();
        tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
        jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1);

        // This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
        const uint16_t cMaxBodies = 1024;

        // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
        const uint16_t cNumBodyMutexes = 0;

        // This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
        // body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
        // too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
        const uint16_t cMaxBodyPairs = 1024;

        // This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
        // number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
        const uint16_t cMaxContactConstraints = 1024;

        // Create mapping table from object layer to broadphase layer
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        auto broad_phase_layer_interface = new BPLayerInterfaceImpl();

        // Create class that filters object vs broadphase layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        auto object_vs_broadphase_layer_filter = new ObjectVsBroadPhaseLayerFilterImpl();

        // Create class that filters object vs object layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        auto object_vs_object_layer_filter = new ObjectLayerPairFilterImpl();

        // Now we can create the actual physics system.
        physicsSystem = new PhysicsSystem();
        physicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *broad_phase_layer_interface, *object_vs_broadphase_layer_filter, *object_vs_object_layer_filter);

        // A body activation listener gets notified when bodies activate and go to sleep
        // Note that this is called from a job so whatever you do here needs to be thread safe.
        // Registering one is entirely optional.
        auto body_activation_listener = new MyBodyActivationListener();
        physicsSystem->SetBodyActivationListener(body_activation_listener);

        // A contact listener gets notified when bodies (are about to) collide, and when they separate again.
        // Note that this is called from a job so whatever you do here needs to be thread safe.
        // Registering one is entirely optional.
        auto contact_listener = new MyContactListener();
        physicsSystem->SetContactListener(contact_listener);

    }

    JPH::PhysicsSystem *KingGame::getPhysicsSystem() {
        return physicsSystem;
    }

    JPH::JobSystemThreadPool *KingGame::getJobSystem() {
        return jobSystem;
    }

    JPH::TempAllocator *KingGame::getTempAllocator() {
        return tempAllocator;
    }

    FBFont *KingGame::getButtonFont() {
        return buttonFont;
    }

    int KingGame::getGold() {
        return gold;
    }

    int KingGame::getStone() {
        return stone;
    }

    int KingGame::getWood() {
        return wood;
    }

    KingGameState::KingGameState(KingGame *game) : kingGame(game) {

    }

    KingGameState::~KingGameState() {

    }
} // king


DefaultGame* getGame() {
    return new king::KingGame();
}