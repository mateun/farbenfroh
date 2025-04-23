#include <stdio.h>
#include "mars_attack.h"
#include "mars_main_menu.h"
#include "animation_editor.h"
#include <thread>
#include <glm/glm.hpp>

// Jolt Physics includes
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Core.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

// End Jolt includes

GameState* gameState = nullptr;

int pauseMenuIndex = 0;
TutorialState getNextTutorialState(TutorialState current);
void updatePhysicsObjects();
glm::vec3 getMuzzleLocation();



void initMeshes(){
    gameState->meshes["tank_body"] = loadMeshFromFile("../assets/tank2_body.gltf");
    gameState->meshes["tank_turret"] = loadMeshFromFile("../assets/tank2_turret.gltf");
    gameState->meshes["wall"] = loadMeshFromFile("../assets/wall.gltf");
    gameState->meshes["ant"] = loadMeshFromFile("../assets/ant.gltf");
    gameState->meshes["barrel"] = loadMeshFromFile("../assets/barrel.gltf");
    gameState->meshes["projectile"] = loadMeshFromFile("../assets/projectile.gltf");
    gameState->meshes["cube"] = loadMeshFromFile("../assets/cube.gltf");
    gameState->meshes["rock1"] = loadMeshFromFile("../assets/rock1.gltf");
    gameState->meshes["wall"] = loadMeshFromFile("../assets/wall.gltf");
//    gameState->meshes["sk_worm"] = loadMeshFromFile("../assets/skeletal_worm.gltf");

}

void initTextures() {
    loadBitmap("../assets/font.bmp", &gameState->font);
    gameState->textures["fps"] = createTextTexture(512, 32);
    gameState->textures["tutorial"] = createTextTexture(512, 32);
    gameState->textures["barrel"] = createTextureFromFile("../assets/barrel-diffuse1024.png");
    gameState->textures["lowpoly"] = createTextureFromFile("../assets/lp_texture.png", ColorFormat::RGBA);
    gameState->textures["smoke"] = createTextureFromFile("../assets/smoke.png");
    gameState->textures["smoke_white"] = createTextureFromFile("../assets/smoke_white.png");
}

void initParticleEffects() {
    ParticleEffect* muzzleEffect = new ParticleEffect();
    muzzleEffect->location = {-3, 0, -2};
    muzzleEffect->initialSpeed = 0.03;
    muzzleEffect->spawnLag = 0.5;
    muzzleEffect->number = 18;
    muzzleEffect->gravityValue = -0.45;
    muzzleEffect->maxLifeInSeconds = 1;
    muzzleEffect->oneShot = true;
    muzzleEffect->mesh = gameState->meshes["cube"];
    muzzleEffect->texture = gameState->textures["smoke_white"];
    muzzleEffect->rotation = glm::vec3{-90, 0, 0};
    muzzleEffect->done = true;
    gameState->particleEffects["muzzle"] = muzzleEffect;

    ParticleEffect* explosionEffect = new ParticleEffect();
    explosionEffect->location = {-3, 0, -2};
    explosionEffect->initialSpeed = 0.09;
    explosionEffect->spawnLag = 0.5;
    explosionEffect->number = 18;
    explosionEffect->gravityValue = -1.45;
    explosionEffect->maxLifeInSeconds = 1.2;
    explosionEffect->oneShot = true;
    explosionEffect->mesh = gameState->meshes["cube"];
    explosionEffect->texture = gameState->textures["smoke"];
    explosionEffect->rotation = glm::vec3{-45, 0, 0};
    explosionEffect->done = true;
    gameState->particleEffects["explosion"] = explosionEffect;

}

void initObstacles() {

    for (int i = 0; i < 4; i++) {
        auto o = new Obstacle();
        o->dimensionAABB = {0.5, 0.5, 0.5};
        o->mesh = gameState->meshes["rock1"];
        o->texture = gameState->textures["lowpoly"];
        o->singleColor = {0.3, 0.3, 0.3, 1};
        if ( i == 0) {
            o->location = {-10, 0, 0};
        } else if (i == 1) {
            o->location = {10, 0, 0};
        } else if ( i == 2) {
            o->location = {-0, 0, -8};
        } else if ( i == 3) {
            o->location ={-0, 0, 8};
        }
        gameState->obstacles.push_back(o);
    }

    // Some smaller decorative rocks
    auto o = new Obstacle();
    o->dimensionAABB = {0.15, 0.15, 0.15};
    o->mesh = gameState->meshes["rock1"];
    o->singleColor =  glm::vec4{0.3, 0.3, 0.3, 1};
    o->rotation = {0, -60, 0};
    o->location = {-1, 0, -7.4 };
    gameState->obstacles.push_back(o);

    o = new Obstacle();
    o->dimensionAABB = {0.25, 0.15, 0.25};
    o->mesh = gameState->meshes["rock1"];
    o->rotation = glm::vec3{0, 45, 0};
    o->singleColor =  glm::vec4{0.3, 0.3, 0.3, 1};
    //o->texture = gameState->textures["lowpoly"];
    o->location = {1.1, 0, -8 };
    gameState->obstacles.push_back(o);

    int counter = 0;
    for (auto o : gameState->obstacles) {
        auto go = new GameObject{};
        go->name = "obstacle_" + std::to_string(counter++);
        go->location = o->location;
        gameState->allGameObjects.push_back(go);
    }
}

BTNodeResult moveSpider(BTBlackboard* blackboard) {
    glm::vec3 playerPos = blackboard->vec3s["player_position"];
    Enemy* myself = (Enemy*)blackboard->objects["myself"];
    glm::vec3 direction = glm::normalize(playerPos- myself->location);
    myself->location += direction * 0.25f * ftSeconds;
    // our assumed standard forward = 0,0,1.
    float angle = acos(glm::dot(glm::normalize(glm::vec3{0, 0, 1.0f}), direction));

    // I don't know why this is the case but it is necessary to work with our Euler based rotation.
    // Otherwise we do not get negative angle values.
    if (direction.x < 0) {
        angle = -angle;
    }
    myself->rotation = {0, glm::degrees(angle), 0};
    return {true};
}

void initEnemies() {
    for (int i = 0; i < 2; i++) {
        auto spider1 = new Enemy{};
        spider1->type = EnemyType::Spider;
        spider1->location = {-7 + (i*2), 0, -12};
        gameState->enemies.push_back(spider1);
        GameObject* go = new GameObject();
        go->name = "spider_" + std::to_string(i);
        go->type = GameObjectType::Enemy;
        go->object = spider1;

        auto bt = new BehaviorTree();
        bt->blackboard->vec3s["player_position"] = gameState->tankLocation;
        bt->blackboard->objects["myself"] = spider1;
        BTNode moveFdTask;
        moveFdTask.type = BTNodeType::TASK;
        moveFdTask.taskFunction = moveSpider;
        bt->root = moveFdTask;
        spider1->behaviorTree = bt;
        go->type = GameObjectType::Enemy;
        go->object = spider1;
        gameState->allGameObjects.push_back(go);
    }





}


// An example contact listener
class TankofantContactListener : public JPH::ContactListener
{
public:
    // See: ContactListener
    virtual JPH::ValidateResult	OnContactValidate(const JPH::Body &inBody1, const JPH::Body &inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult &inCollisionResult) override
    {


        // TODO cast to specific object, e.g. Enemy, etc.
        // TODO think of how do we know what to cast to?

        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void OnContactAdded(const JPH::Body &inBody1, const JPH::Body &inBody2,
                                           const JPH::ContactManifold &inManifold,
                                           JPH::ContactSettings &ioSettings) override
    {

        auto go1 = (GameObject*)inBody1.GetUserData();
        auto go2 = (GameObject*)inBody2.GetUserData();
        if (go1) {
            if (go1->type == GameObjectType::Projectile) {
                // TODO hit something with the projectile
            }
        }
    }

    virtual void OnContactPersisted(const JPH::Body &inBody1, const JPH::Body &inBody2,
                                               const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override
    {
        std::cout << "A contact was persisted" << std::endl;
    }

    virtual void OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) override
    {
        std::cout << "A contact was removed" << std::endl;
    }
};

void initPhysics() {

    JPH::RegisterDefaultAllocator();
    // Create a factory
    JPH::Factory::sInstance = new JPH::Factory();

    // Register all Jolt physics types
    JPH::RegisterTypes();
    JPH::TempAllocatorImpl* temp_allocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
    JPH::JobSystemThreadPool* job_system = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1);

    const uint32_t cMaxBodies = 1024;
    const uint32_t cNumBodyMutexes = 0;
    const uint32_t cMaxContactConstraints = 1024;
    const uint32_t cMaxBodyPairs = 1024;

    BPLayerInterfaceImpl* broadPhaseLayerInterface = new BPLayerInterfaceImpl();
    ObjectVsBroadPhaseLayerFilterImpl* objectVsBroadPhaseLayerFilter = new ObjectVsBroadPhaseLayerFilterImpl;
    ObjectLayerPairFilterImpl* objectLayerPairFilter = new ObjectLayerPairFilterImpl();
    auto contactListener = new TankofantContactListener();

    auto physicsSystem = new JPH::PhysicsSystem();
    physicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs,
                        cMaxContactConstraints, *broadPhaseLayerInterface,
                        *objectVsBroadPhaseLayerFilter,
                        *objectLayerPairFilter);
    physicsSystem->SetContactListener(contactListener);
    gameState->physicsData  = new PhysicsData { physicsSystem, physicsSystem->GetBodyInterface(),
                                                temp_allocator,
                                                job_system};

    // Floor
    JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));
    JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
    JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
    using namespace JPH::literals;
    JPH::BodyCreationSettings floor_settings(floor_shape, JPH::RVec3(0.0_r, -1.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
    JPH::Body *floor = gameState->physicsData->bodyInterface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr
    gameState->physicsData->bodyInterface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);
    gameState->physicsData->physicsSystem->OptimizeBroadPhase();

}

void init() {
    gameState = new GameState();
    gameState->tutorial = new Tutorial();
    gameState->level = Level::MainMenu;

    initPhysics();


    // Camera setup
    gameState->gamePlayCamera = new Camera();
    glm::vec3 camLoc = {0, 10, 2};
    gameState->gamePlayCamera->location = {camLoc.x, camLoc.y, camLoc.z};
    glm::vec3 lookAtTarget = camLoc + gameState->gamePlayLookFwd;
    gameState->gamePlayCamera->lookAtTarget = {lookAtTarget.x, lookAtTarget.y, lookAtTarget.z};
    gameState->gamePlayCamera->type = CameraType::Perspective;
    gameState->shadowMapCamera = new Camera();
    auto shadowCamLoc = camLoc + gameState->shadowCamOffsetToNormalCam;
    gameState->shadowMapCamera->location =  {shadowCamLoc.x, shadowCamLoc.y, shadowCamLoc.z};
    gameState->shadowMapCamera->lookAtTarget = {-2, 0, -2.99};
    gameState->shadowMapCamera->type = CameraType::OrthoGameplay;
    gameState->uiCamera = new Camera();
    gameState->uiCamera->location = {0, 0, 0};
    gameState->uiCamera->lookAtTarget = {0, 0, -1};
    gameState->uiCamera->type = CameraType::Ortho;


    initMeshes();
    initTextures();

    // Load sounds
    gameState->sounds["turret_turn"] = loadSoundFileExt("../assets/sound/turret_move.wav");
    gameState->sounds["tank_shot"] = loadSoundFileExt("../assets/sound/tank_shot.wav");
    gameState->sounds["tank_moving"] = loadSoundFileExt("../assets/sound/tank_moving.wav");

    initEnemies();
    initObstacles();
    initParticleEffects();

    // This is just experimental to handle instanced draw calls.
    // Rendering a big amount of stones around the play field here.
    // Stone instance offsets
    for (int i = 0; i < 500;i++) {
        for (int r = 0; r < 20; r++) {
            glm::vec2 offset = {-20  + i * 9.5, 10 - r * 10};
            gameState->stoneInstanceOffsets.push_back(offset);
        }
    }

    initPhysics();

}

/**
 * Checks if we should move the tutorial state forward or backward or stay put.
 */
void bumpTutorialState() {
    if (gameState->tutorial->state == TutorialState::UPDOWN) {
        if (gameState->tutorial->moveUpDownRecognised == true) {
            gameState->tutorial->state = TutorialState::LEFTRIGHT;
        }
    }
    if (gameState->tutorial->state == TutorialState::LEFTRIGHT) {
        if (gameState->tutorial->moveLeftRightRecognised == true) {
            gameState->tutorial->state = TutorialState::TURRETTURN;
        }
    }
    if (gameState->tutorial->state == TutorialState::TURRETTURN) {
        if (gameState->tutorial->turretTurnRecognised == true) {
            gameState->tutorial->state = TutorialState::SHOOT;
        }
    }
    if (gameState->tutorial->state == TutorialState::SHOOT) {
        if (gameState->tutorial->shootRecognised == true) {
            gameState->tutorial->state = TutorialState::OVER;
        }
    }

}
/**
 * This includes moving the tank and also collision of the
 * tank with static obstacles and moving entities such
 * as bullets and enemies.
 */
void tankMovement() {
    float tankTurnSpeed = 80.0f * ftSeconds;

    float dir = 0;
    float hdir = 0;

    // Recognising key presses for tutorial
    static bool upRec = false;
    static bool downRec = false;
    static bool leftRec = false;
    static bool rightRec = false;
    if (isKeyDown('W')) {
        dir = 1;
        upRec= true;

    }
    if (isKeyDown('S')) {
        dir = -1;
        downRec= true;
    }
    if (upRec ) {
        gameState->tutorial->moveUpDownRecognised = true;
    }

    if (isKeyDown('A')) {
        hdir = -1;
        leftRec = true;
    }
    if (isKeyDown('D')) {
        hdir = 1;
        rightRec = true;
    }
    if (leftRec && rightRec) {
        gameState->tutorial->moveLeftRightRecognised = true;
    }

    static bool firstTime = true;
    if (dir != 0) {
        if (firstTime) {
            //playSound(gameState->sounds["tank_moving"], true);
            firstTime = false;
        }
    }

    gameState->tankYaw -= hdir * tankTurnSpeed;
    gameState->throttlePercent = dir;
    gameState->brakePercent = dir < 0 ? 1 : 0;
    gameState->tankAccel = 0;

    if (gameState->throttlePercent == 1) {
        gameState->tankAccel = 4.0f * ftSeconds;

    } else if (gameState->throttlePercent == -1) {
        gameState->tankAccel = -0.3f * ftSeconds;
    }
    else if (gameState->throttlePercent == 0) {

        // Active Braking - TODO
//        if (gameState->tankSpeed > 0) {
//            gameState->brakePercent = 1;
//            gameState->tankAccel = -12.0f * ftSeconds;
//            if (gameState->tankSpeed < 0) {
//                gameState->tankSpeed = 0;
//            }
//        } else if (gameState->tankSpeed < 0) {
//            gameState->brakePercent = -1;
//            gameState->tankAccel = 12.0f * ftSeconds;
//            if (gameState->tankSpeed > 0) {
//                gameState->tankSpeed = 0;
//            }
//        }
        // Engine brake
        gameState->tankSpeed -= 0.07 * ftSeconds;
    }

    gameState->tankSpeed += gameState->tankAccel;
    if (gameState-> tankSpeed > 5 * ftSeconds) {
        gameState->tankSpeed = 5 * ftSeconds;
    }

    if (gameState->tankSpeed < 0) {
        gameState->tankSpeed = 0;
    }

    auto oldLocation = gameState->tankLocation;
    glm::vec4 tankFwd = {0, 0, -1, 1};
    auto matYaw= glm::rotate(glm::mat4(1), glm::radians(gameState->tankYaw), {0, 1, 0});
    tankFwd = glm::normalize(matYaw * tankFwd);
    auto potentialNextLocation = gameState->tankLocation + glm::vec3{tankFwd.x, tankFwd.y, tankFwd.z} * gameState->tankSpeed;

    // Collision check with static obstacles
    for (auto o : gameState->obstacles) {
        float distance = glm::distance(potentialNextLocation, o->location);
        if (distance < 1) {
            potentialNextLocation = gameState->tankLocation;
        }
    }

    //gameState->tankLocation += glm::vec3{tankFwd.x, tankFwd.y, tankFwd.z} * gameState->tankSpeed * .03f;
    gameState->tankLocation = potentialNextLocation;

    // Move the camera with the tank
    auto camLoc = gameState->tankLocation + glm::vec3{0, 18, 3};
    gameState->gamePlayCamera->location = {camLoc.x, camLoc.y, camLoc.z};
    auto tgt = camLoc + gameState->gamePlayLookFwd;
    gameState->gamePlayCamera->lookAtTarget = {tgt.x, tgt.y, tgt.z};

    // Move the light/shadwmap with the camera
    auto smFwd = camFwd(gameState->shadowMapCamera);
    glm::vec3 smLoc = gameState->gamePlayCamera->location + gameState->shadowCamOffsetToNormalCam;
    gameState->shadowMapCamera->location = {smLoc.x, smLoc.y, smLoc.z};
    auto smLookTarget = smLoc + glm::vec3{smFwd.x, smFwd.y, smFwd.z};
    gameState->shadowMapCamera->lookAtTarget = {smLookTarget.x, smLookTarget.y, smLookTarget.z};


    // Store tank positions if moving
    gameState->tankPositions.clear();
    if (abs(gameState->tankSpeed) != 0) {
        gameState->tankPositions.push_back(oldLocation);
    }

    gameState->turretLocation = gameState->tankLocation + gameState->turretLocationOffset;


}

void cameraMovement() {
    float camspeed = 2 * ftMicros / 1000.0f / 1000.0f;
    float dir = 0;
    float hdir = 0;
    if (isKeyDown('W')) {
        dir = 1;
    }
    if (isKeyDown('S')) {
        dir = -1;
    }

    if (isKeyDown('A')) {
        hdir = -1;
    }
    if (isKeyDown('D')) {
        hdir = 1;
    }


    auto fwd = camFwd(gameState->gamePlayCamera) ;
    auto right = cameraRightVector(gameState->gamePlayCamera);
    glm::vec3 loc = gameState->gamePlayCamera->location;
    glm::vec3 horizontalMovement= glm::vec3{camspeed * fwd.x, 0, camspeed * fwd.z} * dir;
    glm::vec3 verticalMovement = glm::vec3{camspeed * right.x   , 0, camspeed * right.z } * hdir;
    loc += horizontalMovement;
    loc += verticalMovement;
    auto tgt = loc + glm::vec3{fwd.x, fwd.y, fwd.z};
    gameState->gamePlayCamera->location = {loc.x, loc.y, loc.z};
    gameState->gamePlayCamera->lookAtTarget = {tgt.x, tgt.y, tgt.z};

    // Also move the directional shadow light with
    // the camera so we always have shadows
    auto smFwd = camFwd(gameState->shadowMapCamera);
    auto smRight = cameraRightVector(gameState->shadowMapCamera);
    glm::vec3 smLoc = gameState->shadowMapCamera->location;
    smLoc += horizontalMovement;
    smLoc += verticalMovement;
    auto smLookTarget = smLoc + glm::vec3{smFwd.x, smFwd.y, smFwd.z};
    gameState->shadowMapCamera->location = {smLoc.x, smLoc.y, smLoc.z};
    gameState->shadowMapCamera->lookAtTarget = {smLookTarget.x, smLookTarget.y, smLookTarget.z};

}

void updatePhysicsObjects() {
    float cDeltaTime = 1.0f/60.0f;
    for (auto go : gameState->allGameObjects) {
        if (go->usePhysics) {
           if (gameState->physicsData->bodyInterface.IsActive(go->physicsBody->GetID())) {
               JPH::RVec3 position = gameState->physicsData->bodyInterface.GetCenterOfMassPosition(go->physicsBody->GetID());
               JPH::Vec3 velocity = gameState->physicsData->bodyInterface.GetLinearVelocity(go->physicsBody->GetID());
               go->location = {position.GetX(), position.GetY() - 0.2f, position.GetZ()};
               printf("loc: %f/%f/%f\n", go->location.x, go->location.y, go->location.z);
               gameState->physicsData->physicsSystem->Update(cDeltaTime, 1,
                                                             gameState->physicsData->tempAllocator,
                                                             gameState->physicsData->jobSystem);
           }

        }
    }

}

void updateEnemies() {
    std::vector<Enemy*> toBeDeleted;
    for (auto e : gameState->enemies) {

        // First check if an enemy has been hit
        // For now remove this enemy
        // TODO deduct from HP etc.
        if (e->markAsHit) {
            toBeDeleted.push_back(e);
        }

        // First some updates to the blackboard.
        e->behaviorTree->blackboard->vec3s["player_position"] = gameState->tankLocation;
        e->behaviorTree->execute();
    }
    for (auto ed : toBeDeleted) {
        int index = 0;
        for (auto x : gameState->enemies) {
            if (ed == x) {
                break;
            }
            index++;
        }

        gameState->enemies.erase(gameState->enemies.begin() + index);

        // Also erase from game object list
        index = 0;
        for (auto x : gameState->allGameObjects) {
            if (ed == x->object) {
                break;
            }
            index++;
        }
        gameState->allGameObjects.erase(gameState->allGameObjects.begin() + index);
    }



}

/**
 * Get the normalized turret direction vector (in world space)
 * @return
 */
glm::vec3 getTurretDir(){
    glm::mat4 mrot = glm::rotate(glm::mat4(1), glm::radians(gameState->turretYaw), {0, 1, 0});
    return glm::normalize(mrot * glm::vec4{0, 0, -1, 1});
}

/**
 * Calculate the current muzzle world position
 * based on the position and rotation of the turret.
 */
glm::vec3 getMuzzleLocation() {
    return gameState->tankLocation + getTurretDir() + glm::vec3{0, 0.75, 0};
}

void shooting() {
    float ftSecs = ftMicros / 1000.0f / 1000.0f;
    if (keyPressed(VK_SPACE))
    {
        static int projectileCount = 0;
        projectileCount++;
        gameState->tutorial->shootRecognised = true;
        Projectile* p = new Projectile();
        p->id = "projectile_" + std::to_string(projectileCount);
        p->location = getMuzzleLocation();
        p->velocity = getTurretDir() * 22.0f;
        p->rotation = {0, gameState->turretYaw, 0};
        gameState->projectiles.push_back(p);
        GameObject* go = new GameObject {p->id, GameObjectType::Projectile, p->location};
        go->object = p;
        gameState->allGameObjects.push_back(go);

        // Run a particle emitter once
        resetParticleEffect(gameState->particleEffects["muzzle"]);
        gameState->particleEffects["muzzle"]->location = p->location;
        gameState->doTurretRecoil = true;

    }

    std::vector<Projectile*> toBeDeleted;
    for (auto p: gameState->projectiles) {
        p->lifeTime += ftSecs;
        if (p->lifeTime > p->maxLife || p->markDelete) {
            toBeDeleted.push_back(p);
            continue;
        }
        p->location += p->velocity * ftSecs;
    }

    for (auto p : toBeDeleted) {
        int index = 0;
        for (auto x : gameState->projectiles) {
            if (x == p) {
                break;
            }
            index++;
        }
        gameState->projectiles.erase(gameState->projectiles.begin() + index);

        // Remove from overall game objects list as well
        int goIndex = 0;
        for (auto x : gameState->allGameObjects) {
            if (x->name == p->id) {
                gameState->allGameObjects.erase(gameState->allGameObjects.begin() + goIndex);
                break;
            }
            goIndex++;
        }
    }


}

void turretTurn() {
    auto ftSecs = (float)ftMicros / 1000.0 / 1000.0;
    float rotSpeed = 150;

    static bool lturn = false;
    static bool rturn = false;
    if (isKeyDown(VK_LEFT)) {
        gameState->turretYaw += (ftSecs * rotSpeed);
        lturn = true;
    }

    if (isKeyDown(VK_RIGHT)) {
        gameState->turretYaw -= (ftSecs * rotSpeed);
        rturn = true;
    }
    if (lturn && rturn) {
        gameState->tutorial->turretTurnRecognised = true;
    }

    if (lturn || rturn) {
        //playSound(gameState->sounds["turret_turn"], false);
    }
}

void updateParticleEffects() {

    for (auto pe : gameState->particleEffects) {
        updateParticleEffect(pe.second);
    }
}

void checkCollisions() {
    for (auto p : gameState->projectiles) {

        // Check obstacle collisions
        for (auto o : gameState->obstacles) {
            auto distance = glm::distance(p->location, o->location);
            if (distance < 1 ) {
                if (o->destructable && !o->markAsHit) {
                    o->markAsHit = true;
                }
                else {
                    p->markHit = true;
                }
            }
        }

        // Check enemy collisions
        for (auto o : gameState->enemies) {
            auto distance = glm::distance(p->location, o->location);
            if (distance < 1 ) {
                o->markAsHit = true;
                p->markHit = true;
            }
        }
    }
}

void updateObstacles() {
    std::vector<int> toRemoveList;
    for (int i = 0; i< gameState->obstacles.size(); i++) {
        auto o = gameState->obstacles[i];
        if (o->toRemove) {
            toRemoveList.push_back(i);
        }
    }

    for (auto index : toRemoveList) {
        gameState->obstacles.erase(gameState->obstacles.begin() + index);
    }


}

/**
 * We go back and forth with the turret when recoiling.
 */
void updateRecoilAnim() {
    if (gameState->doTurretRecoil) {
        gameState->recoilAnimDuration += ftSeconds;
        static bool reversed = false;
        if (gameState->recoilAnimDuration >= gameState->recoilAnimMaxDuration) {
            gameState->recoilAnimDuration = 0;

            if (!reversed) {
                // This is our first time around
                reversed = true;
            } else {
                gameState->doTurretRecoil = false;
                reversed = false;
                gameState->turretLocationOffset = {0, 0, 0};
                return;
            }
        }
        // What is the forward direction of the turret?
        if (reversed) {
            gameState->turretLocationOffset += (getTurretDir() * ftSeconds * 1.0f);
        } else {
            gameState->turretLocationOffset -= (getTurretDir() * ftSeconds * 1.0f);
        }

    }


}
void updatePauseMenu() {
    if (keyPressed(VK_DOWN))
    {
        pauseMenuIndex ++;
        if (pauseMenuIndex > 2) {
            pauseMenuIndex = 2;
        }
    }

    if (keyPressed(VK_UP))
    {
        pauseMenuIndex--;
        if (pauseMenuIndex < 0) {
            pauseMenuIndex = 0;
        }
    }

    if (keyPressed(VK_RETURN))
    {
        if (pauseMenuIndex == 0) {
            gameState->level = Level::Gameplay;
        }

        if (pauseMenuIndex == 1) {
            gameState->level = Level::MainMenu;
        }

        if (pauseMenuIndex == 2) {
            gameState->shouldRun = false;
        }
    }
}

void spawnPhysicsObject() {
    using namespace JPH::literals;
    JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.1f), JPH::RVec3(0.0_r, 5.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
    //JPH::BodyID sphere_id = gameState->physicsData->bodyInterface.CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);
    auto body = gameState->physicsData->bodyInterface.CreateBody(sphere_settings);
    auto enemy = new Enemy();
    enemy->location = {0, 5, 0};

    gameState->physicsData->bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);
    gameState->physicsData->bodyInterface.SetLinearVelocity(body->GetID(), JPH::Vec3(0.0f, -5.0f, 0.0f));
    auto go = new GameObject();
    go->name = "physics_object-1";
    go->location =  {0, 5, 0};
    go->usePhysics = true;
    go->physicsBody = body;
    body->SetUserData((uint64_t)go);
    gameState->allGameObjects.push_back(go);
    if (gameState->physicsData->bodyInterface.IsActive(go->physicsBody->GetID())) {
        printf("active!\n");
    }
}

void update(){
    auto ftSecs = (float)ftMicros / 1000.0 / 1000.0;
    if (keyPressed(VK_ESCAPE))
    {
        gameState->level = Level::PauseMenu;
    }
    if (keyPressed('E')) {
        gameState->level = Level::AnimEditor;
    }

    if (keyPressed('P')) {
        spawnPhysicsObject();
    }

    if (keyPressed('Q')) {
        gameState->level = Level::InGameEditor;
        gameState->paused = true;
    }

    if (gameState->level == Level::PauseMenu) {
        updatePauseMenu();
    } else if(gameState->level == Level::AnimEditor) {
        updateAnimEditor();
    } else {
        updateInGameEditor();
        if (gameState->paused) return;

        turretTurn();
        tankMovement();
        updateEnemies();
        updatePhysicsObjects();
        updateObstacles();
        shooting();
        updateRecoilAnim();
        checkCollisions();
        updateParticleEffects();
        bumpTutorialState();
    }

}

TutorialState getNextTutorialState(TutorialState current) {
    if (current == TutorialState::UPDOWN) {
        return TutorialState::LEFTRIGHT;
    }
    if (current == TutorialState::LEFTRIGHT) {
        return TutorialState::TURRETTURN;
    }
    if (current == TutorialState::TURRETTURN) {
        return TutorialState::SHOOT;
    }
    return TutorialState::OVER;
}

std::string getTutorialText(TutorialState tutorialState) {
    if (tutorialState == TutorialState::UPDOWN) {
        return gameState->tutorial->movUpDownText;
    }

    if (tutorialState == TutorialState::LEFTRIGHT) {
        return gameState->tutorial->moveLeftRightText;
    }

    if (tutorialState == TutorialState::TURRETTURN) {
        return gameState->tutorial->turretTurnText;
    }

    if (tutorialState == TutorialState::SHOOT) {
        return gameState->tutorial->shootText;
    }
    return "";
}

void renderTutorial() {
    if (gameState->tutorial->state == TutorialState::OVER) {
        return;
    }

    foregroundColor({.8, .8, .8, 1});
    auto text = getTutorialText(gameState->tutorial->state);
    char buf[150];
    sprintf_s(buf, 150, text.c_str());
    font(gameState->font);
    textScale({3, 3});
    updateAndDrawText(buf,
                      gameState->textures["tutorial"],
                      window_width / 2 + (gameState->textures["tutorial"]->bitmap->width / 2),
                      window_height / 2 - 10);
    textScale({1, 1});
}

void renderTank() {
    bindTexture(gameState->textures["lowpoly"]);
    flipUvs(false);

    scale({1, 1, 1});
    bindMesh(gameState->meshes["tank_body"]);

    location(gameState->tankLocation + glm::vec3(0, 0, 0));
    rotation({0, gameState->tankYaw, 0});
    drawMesh();

    bindMesh(gameState->meshes["tank_turret"]);
    rotation({0, gameState->turretYaw, 0});
    location(gameState->turretLocation);
    drawMesh();

    bindTexture(nullptr);

}

void renderProjectiles() {
    for (auto p : gameState->projectiles) {
        if (p->markHit) {
            resetParticleEffect(gameState->particleEffects["explosion"]);
            gameState->particleEffects["explosion"]->location = p->location;
            p->markDelete = true;
        } else {
            bindMesh(gameState->meshes["projectile"]);
            bindTexture(nullptr);
            scale({0.2, 0.2, 0.2});
            foregroundColor({0.9, 0.3, 0.1, 1});
            location(p->location);
            rotation(glm::vec(p->rotation));
            drawMesh();
            rotation({0, 0, 0});
        }
    }
}

void renderObstacles() {
    for (auto o : gameState->obstacles) {
        if (o->markAsHit) {
            resetParticleEffect(gameState->particleEffects["muzzle"]);
            gameState->particleEffects["muzzle"]->location = o->location;
            o->toRemove = true;
        } else {
            bindMesh(o->mesh);
//            bindTexture(o->texture);
            foregroundColor(o->singleColor);
            location(o->location);
            scale(o->dimensionAABB);
            rotation(o->rotation);
            drawMesh();
        }

    }
}

void renderParticles() {
    for (auto pe : gameState->particleEffects) {
        if (!pe.second->done) {
            bindParticleEffect(pe.second);
            drawParticleEffect();
        }
    }
}

void renderEnemies() {
    for (auto e : gameState->enemies) {
        // Only draw enemies in front of the camera
        if (e->location.z < gameState->gamePlayCamera->location.z) {
            bindMesh(gameState->meshes["ant"]);
            scale({0.2, 0.4, 0.2});
            foregroundColor({0.2, 0.6, 0.1, 1});
            location(e->location);
            rotation(e->rotation);
            drawMesh();
        }

    }

    // Temp: Render physics objects here for now
    for (auto go : gameState->allGameObjects) {
        if (go->usePhysics) {
            bindMesh(gameState->meshes["ant"]);
            scale({0.1, 0.1, 0.1});
            foregroundColor({0.2, 0.6, 0.1, 1});
            location(go->location);
            drawMesh();
        }
    }

    // Try render the skeletal worm
//    bindMesh(gameState->meshes["sk_worm"]);
//    foregroundColor({0, 0.8, 0, 1});
//    location({-3, 0, -4});
//    scale({1, 1, 1});
//    drawMesh();
}

void renderBarrels() {
    bindTexture(gameState->textures["barrel"]);
    bindMesh(gameState->meshes["barrel"]);
    scale({0.25, 0.25, 0.25});
    for (int i = 0; i < 5; i++) {
        location(glm::vec3{-3 + i * 1.5, 0, -3});
        drawMesh();
    }
}

void renderStones() {
    bindTexture(gameState->textures["lowpoly"]);
    bindMesh(gameState->meshes["wall"]);
    scale({0.05, 0.03, 0.05});
    foregroundColor({0.45, 0.2, 0.1, 1});
    location (glm::vec3{0, 0, 0});
    instanceOffsets(gameState->stoneInstanceOffsets);
    //drawMeshInstanced(10000);
    bindTexture(nullptr);

}

void renderGroundPlane() {
    bindTexture(nullptr);
    location({0, -0.05, 0});
    scale({40, 40, 1});
    rotation({-90, 0, 0});
    foregroundColor({.6, 0.3, 0.3, 1});
    drawPlane();
    rotation({0, 0,0});

}

void renderPauseMenu() {
    // Draw the pause menu on top of everything,
    // if needed
    if (gameState->level == Level::PauseMenu) {
        shadowOff();
        tint({1, 1,1,1});

        foregroundColor({0.1, 0.1, 0.1, 1});
        location({window_width/2, window_height/2+24, -0.69});
        scale({260, 124, 1});
        drawPlane();

        tilingOn(true);
        flipUvs(true);
        bindTexture(gameState->textures["menu_items"]);

        // Title
        tileData(3, 0, 128, 64 );
        location({window_width/2, 485, -.5});
        scale({128, 64, 1});
        drawPlane();


        // Continue gameplaye item
        tileData(0, 2, 64, 32, 9*32, 0 );
        location({window_width/2, 420, -.5});
        scale({64*1.5, 32*1.5, 1});
        if (pauseMenuIndex == 0){
            tint({1, 0, 0, 1});
        }
        drawPlane();
        tint({1, 1,1,1});

        // Exit to main menu
        tileData(0, 2, 128, 32, 64, 0 );
        location({window_width/2, 385, -.5});
        scale({128*1.5, 32*1.5, 1});
        if (pauseMenuIndex == 1){
            tint({1, 0, 0, 1});
        }
        drawPlane();
        tint({1, 1,1,1});

        // Extit to desktop
        tileData(0, 2, 96, 32, 64+128, 0 );
        location({window_width/2, 350, -.5});
        scale({96*1.5, 32*1.5, 1});
        if (pauseMenuIndex == 2){
            tint({1, 0, 0, 1});
        }
        drawPlane();
        tint({1, 1,1,1});

        // Cursor
        tileData(0, 1, 64, 64);
        tintRed();
        scale({32*1.5, 32*1.5, 1});
        location({window_width/2 - 105, 420 - (35*pauseMenuIndex), -.5});
        drawPlane();

        tilingOn(false);
        flipUvs(false);
    }
}

void renderUI2D() {
    rotation({0, 0, 0});
    bindCamera(gameState->uiCamera);
    lightingOff();

    // FPS Text rendering
    {
        foregroundColor({.1, .7, .1, 1});
        char buf[160];
        sprintf_s(buf, 160, "FT:%6.1fmcs %4d/%4d Tankspd: %3.2f brk: %3.2f",
                  ftMicros, mouse_x, mouse_y, gameState->tankSpeed, gameState->brakePercent);
        font(gameState->font);
        updateAndDrawText(buf, gameState->textures["fps"], gameState->textures["fps"]->bitmap->width / 2 + 2,
                          5);
    }

    renderTutorial();
    renderPauseMenu();



}

void renderTankTrackMarks() {
    // Render tank track marks
    // This costs a lot of performance,
    // even when not issuing draw calls.
    // Commented out for now.
    // We generate a lot of tank positions here and the
    // iteration eats us up.
    static float dur = 0;
    dur += ftSeconds;
//    for (auto tl : gameState->tankPositions) {
////        location(tl + glm::vec3{0.2, 0, 0.5});
////        scale({0.2, .2, 2});
////        foregroundColor({0.5, 0.2, 0, 1});
////       // drawPlane();
////        location(tl + glm::vec3{-0.2, 0, 0.5});
////       // drawPlane();
//
////        if (dur > 1.25) {
////            resetParticleEffect(gameState->particleEffects["explosion"]);
////            gameState->particleEffects["explosion"]->location = tl + glm::vec3{0.2, 0, 0.5};
////            dur = 0;
////        }
//
//
//    }
}

void renderWalls() {
    bindTexture(nullptr);
    location({0, -0.05, -20});
    scale({2, 1, 2});
    //rotation({-90, 0, 0});
    foregroundColor({.4, 0.4, 0.4, 1});
    bindMesh(gameState->meshes["wall"]);
    drawMesh();

    scale({2, 1, 2});
    location({-20, 0, 0});
    rotation({0, 90,0});
    drawMesh();
}

void render() {
    bindCamera(gameState->gamePlayCamera);
    bindShadowMapCamera(gameState->shadowMapCamera);

    if (gameState->level == Level::PauseMenu) {
        tint({0.3, 0.3, 0.3, 0.5});
    } else {
        tint({1, 1, 1, 1});
    }

    scale({1, 1,1});
    foregroundColor({0.3, 0.6, 0.2, .1});
    location(glm::vec3{0, 0, 0});
    gridLines(100);
    //drawGrid();

    lightingOn();
    shadowOn();
    deferredStart();

    renderTank();
    renderEnemies();
    renderProjectiles();
    renderObstacles();
    renderBarrels();
    renderGroundPlane();
    renderWalls();

    shadowOff();

    renderStones();
    renderTankTrackMarks();
    renderParticles();

    deferredEnd();
    renderUI2D();

    renderInGameEditor();

}


bool gameLoop() {
    static bool initalized = false;
    if (!initalized) {
        init();
        initalized = true;
    }
    if (gameState->level == Level::MainMenu) {
        updateMainMenu();
        renderMainMenu();
    }
    else if (gameState->level == Level::AnimEditor) {
        updateAnimEditor();
        renderAnimEditor();
    } else {
        update();
        render();
    }

    return gameState->shouldRun;
}