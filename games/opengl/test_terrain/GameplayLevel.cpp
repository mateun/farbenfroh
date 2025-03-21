//
// Created by mgrus on 02.03.2025.
//

#include "GameplayLevel.h"

#include "EnemyExplosionComponent.h"
#include "PlayerBulletPool.h"
#include "PlayerShooting.h"
#include  "../../../src/engine/game/default_game.h"
class gru::ParticleSystem;
class gru::ParticleEmitter;

namespace ttg {
    GameplayLevel::GameplayLevel(DefaultGame *game, const std::string& name): GameLevel(game, name) {
    }

    void GameplayLevel::renderShadowBias() {
        if (scene->getLightsOfType(LightType::Directional).empty()) return;

        static FBFont* font;
        if (!font) {
            font = new FBFont("../assets/font.bmp");
        }

        lightingOff();
        bindCamera(game->getUICamera());
        char buf[175];
        sprintf_s(buf, 160, "shadow bias:%1.5f",
                  scene->getLightsOfType(LightType::Directional)[0]->shadowBias);
        flipUvs(false);
        font->renderText(buf, {2, scaled_height -66, 0.9});
    }

    void GameplayLevel::renderPlayerStats() {
        static FBFont* font;
        if (!font) {
            font = new FBFont("../assets/font.bmp");
        }

        lightingOff();
        bindCamera(game->getUICamera());
        char buf[175];



        auto padNode = scene->findNodeById("heroPad");

        sprintf_s(buf, 160, "# player location:%3.2f/%3.2f/%3.2f",
                  padNode->getLocation().x, padNode->getLocation().y, padNode->getLocation().z);
        font->renderText(buf, {2, scaled_height - 102, 0.87});

        sprintf_s(buf, 160, "# cam distance:%3.2f",
          distance(cameraNode->getLocation(), padNode->getLocation()));
        font->renderText(buf, {2, scaled_height - 142, 0.87});



    }

    void GameplayLevel::render() {
        if (keyPressed('B')) {
            scene->getLightsOfType(LightType::Directional)[0]->shadowBias += 0.0001f;
        }
        if (keyPressed('V')) {
            scene->getLightsOfType(LightType::Directional)[0]->shadowBias -= 0.0001f;
        }
        scene->render();


        //psystem0->render(inFlyCamDebugMode ? scene->getDebugFlyCam() : game->getGameplayCamera());
        // peSmoke0->draw((inFlyCamDebugMode ? scene->getDebugFlyCam() : game->getGameplayCamera()));
        // peSmoke1->draw((inFlyCamDebugMode ? scene->getDebugFlyCam() : game->getGameplayCamera()));
        // peSmoke2->draw((inFlyCamDebugMode ? scene->getDebugFlyCam() : game->getGameplayCamera()));

        // // Active enemy explosions
        // {
        //     for (auto ee: enemyExplosionParticles) {
        //         ee->render(game->getGameplayCamera());
        //     }
        // }

        game->renderFPS();
        renderShadowBias();
        renderPlayerStats();
    }



    // void GameplayLevel::updateActiveEnemyExplosions() {
    //     for (auto ee: enemyExplosionParticles) {
    //         ee->update();
    //     }
    // }

    // Should no longer be needed, only kept for reference
    // until playerBulletComponent is fully ready.
    // void GameplayLevel::updatePlayerBullets() {
    //     for (auto b : playerBulletPool) {
    //         if (!b->isActive()) {
    //             continue;
    //         }
    //
    //         auto pos = b->getLocation();
    //         pos += b->getForwardVector() * ftSeconds * 45.0f;
    //         b->setLocation(pos);
    //         BulletData* bd = (BulletData*) b->getExtraData();
    //         bd->currentLifeInSeconds += ftSeconds;
    //         if (bd->currentLifeInSeconds >= bd->maxLifeInSeconds) {
    //             b->disable();
    //             bd->currentLifeInSeconds = 0;
    //         }
    //
    //         // Check for collision with enemies
    //         for (auto e: enemyList) {
    //             if (!e->isActive()) {
    //                 continue;
    //             }
    //             float distance = glm::distance(pos, e->getLocation());
    //             if (distance < 2) {
    //                 b->disable();
    //                 e->disable();
    //                 auto explosionComp = (SceneNode*) e->getExtraData();
    //                 explosionComp->enable();
    //             }
    //         }
    //
    //         // TODO check wall collision
    //     }
    // }

    void GameplayLevel::checkPlayerCollision() {
        auto padNode = scene->findNodeById("heroPad");

        // First check all walls
        // left
        if (padNode->getLocation().x < -22.5f) {
            padNode->setLocation({-22.5f, padNode->getLocation().y, padNode->getLocation().z});
        }
        // right
        if (padNode->getLocation().x > 24.0f) {
            padNode->setLocation({24.0f, padNode->getLocation().y, padNode->getLocation().z});
        }
    }

    void GameplayLevel::cameraUpdate() {
        auto padNode = scene->findNodeById("heroPad");

        // We want the camera to follow the player, but with a smooth lag.
        // This is the ideal position we want to be in.
        // If the player moves away from us, we try to catch up to him.
        glm::vec3 targetPosition = padNode->getLocation() + glm::vec3{0, 34, 8};
        float currentDelta = glm::distance(cameraNode->getLocation(), targetPosition);
        float maxDelta = 5.0f;
        if (currentDelta > maxDelta) {
            glm::vec3 direction = glm::normalize(targetPosition - cameraNode->getLocation());
            glm::vec3 pos = cameraNode->getLocation();
            pos += glm::vec3{direction.x, 0, direction.z} *ftSeconds * 20.0f;
            cameraNode->setLocation(pos);
            cameraNode->getCamera()->updateLookupTarget({cameraNode->getLocation().x, cameraNode->getLocation().y - 34, cameraNode->getLocation().z -8 });
            // cam->updateLocation({0,34, 8});
            // cam->updateLookupTarget({0, 0, 1});
        }
    }

    void GameplayLevel::update() {

#ifdef USE_POINT_LIGHTS
        auto plNode1 = scene->findNodeById("pointLight1");
        // TODO maybe move this into the actual node, so
        // each node has some logic component which takes over the actual movement etc.
        // Or we do it externally.. not sure, what is actually better. Encapsulation favors
        // the script component.
        static float timePassed= 0;
        timePassed += ftSeconds;
        plNode1->setLocation({-5 + (10 * sinf(timePassed)), 5, 2});
#endif

        scene->update();
        if (keyPressed(VK_F11)) {
            inFlyCamDebugMode = !inFlyCamDebugMode;
            scene->activateDebugFlyCam(inFlyCamDebugMode);
        }

        if (!inFlyCamDebugMode) {
            //cameraMover->update();
            characterController->update();


            // TODO: do we check against invisible walls on our floating platforms?!
            // Currently this is somewhere checked, not specifically the bounds of a platform,
            // but this would be a good idea, I guess.
            checkPlayerCollision();
            cameraUpdate();

            playerShootingLogic->update();

            // TODO bullet movement and collision logic



        }

    }

    void GameplayLevel::init() {

        basicShader = std::make_unique<Shader>();
        basicShader->initFromFiles("../games/opengl/test_terrain/assets/shaders/basic.vert", "../games/opengl/test_terrain/assets/shaders/basic.frag");
        basicShaderUnlit = new Shader();
        basicShaderUnlit->initFromFiles("../games/opengl/test_terrain/assets/shaders/basic.vert", "../games/opengl/test_terrain/assets/shaders/basic_unlit.frag");
        emissiveShader = new Shader();
        emissiveShader->initFromFiles("../src/engine/fx/shaders/emissive.vert", "../src/engine/fx/shaders/emissive.frag");

        terrain = new Terrain(50, 50);

        MeshDrawData smd;

        auto terrainNode = std::make_unique<SceneNode>("terrain");
        //terrainNode->disable();
        //smd.mesh = terrain->getMesh();
        smd.mesh = game->getMeshByName("cube_ground2");
        terrainNode->setScale({2, 1, 2});
        terrainNode->setLocation({0, -1, 0});
        smd.shader = basicShader.get();
        smd.texture = game->getTextureByName("ground_albedo");
        smd.normalMap = game->getTextureByName("ground_normal");
        smd.uvScale2 = {60, 60};
        smd.normalUVScale2 = {200, 200};
        smd.uvScale = 1;
        terrainNode->initAsMeshNode(smd);

        auto subTerrain = std::make_unique<SceneNode>("terrain");
        //terrainNode->disable();
        //smd.mesh = terrain->getMesh();
        smd.mesh = game->getMeshByName("cube_ground");
        subTerrain->setScale({0.5, 2, 0.5});
        subTerrain->setLocation({28, -8.5, -20});
        smd.shader = basicShader.get();
        smd.texture = game->getTextureByName("ground_albedo");
        smd.tint = {1.5, 0.5, 1, 1};
        smd.color = {0,0, 1, 1};
        smd.normalMap = game->getTextureByName("ground_normal");
        smd.uvScale2 = {30, 30};
        smd.uvScale = 1;
        subTerrain->initAsMeshNode(smd);

        auto grassNode1 = std::make_unique<SceneNode>("grassPart1");
        smd.mesh = game->getMeshByName("grass_field");
        grassNode1->setScale({4, 5,4});
        grassNode1->setLocation({4, -0.15, -3});
        smd.shader = basicShader.get();
        smd.texture = game->getTextureByName("grass_diffuse");
        smd.uvScale2 = {4, 4};
        smd.uvScale = 1;
        grassNode1->initAsMeshNode(smd);

        auto grassNode2 = std::make_unique<SceneNode>("grassPart2");
        smd.mesh = game->getMeshByName("grass_field");
        grassNode2->setScale({4, 4.2,4});
        grassNode2->setLocation({-14, -0.15, 2});
        grassNode2->setOrientation(glm::angleAxis(glm::radians(14.0f), glm::vec3(0, 1, 0)));
        grassNode2->initAsMeshNode(smd);

        auto stoneFieldNode1 = std::make_unique<SceneNode>("grassPart1");
        stoneFieldNode1->setScale({1, 1,1 });
        stoneFieldNode1->setLocation({-8, -0.12, 0});
        //grassNode2->setOrientation(glm::angleAxis(glm::radians(34.0f), glm::vec3(0, 1, 0)));
        smd.mesh = game->getMeshByName("stone_field");
        smd.tint = {1, 1, 1, 1};
        smd.texture = game->getTextureByName("asphalt_albedo");
        smd.normalMap = game->getTextureByName("asphalt_normal");
        smd.shader = basicShader.get();
        smd.uvScale2 = {1, 1};
        smd.uvScale = 1;
        stoneFieldNode1->initAsMeshNode(smd);

        auto stoneFieldNode2 = std::make_unique<SceneNode>("grassPart1");
        stoneFieldNode2->setScale({1, 1,1 });
        stoneFieldNode2->setLocation({8, -0.12, 3});
        stoneFieldNode2->initAsMeshNode(smd);

        auto roadNode = std::make_unique<SceneNode>("road");
        smd.mesh = game->getMeshByName("road_plane");
        smd.texture = game->getTextureByName("asphalt_albedo");
        smd.normalMap = game->getTextureByName("asphalt_normal");
        smd.uvScale2 = {1, 5};
        smd.uvScale = 1;
        smd.tint = {1,1, 1, 1};
        smd.uvPan = {0, 0};
        roadNode->setScale({1, 1, 5});
        roadNode->setLocation({0, 0.001, -1});
        roadNode->initAsMeshNode(smd);

        auto roadNode2 = new SceneNode("road2");
        smd.mesh = game->getMeshByName("road_plane_bent");
        smd.texture = game->getTextureByName("asphalt_albedo");
        smd.normalMap = game->getTextureByName("asphalt_normal");
        smd.uvScale2 = {1, 5};
        smd.uvScale = 1;
        smd.uvPan = {0, 0.4};
        roadNode2->setScale({1, 1, 5});
        roadNode2->setLocation({1.01, 0.001, -1});
        roadNode2->initAsMeshNode(smd);

        auto roadBarrier = new SceneNode("roadBarrier");
        smd.mesh = game->getMeshByName("road_barrier");
        smd.texture = game->getTextureByName("road_barrier_albedo");
        smd.normalMap = game->getTextureByName("road_barrier_normal");
        smd.uvScale2 = {1, 1};
        smd.uvScale = 1;
        smd.uvPan = {0, 0};
        roadBarrier->setScale({0.5, 0.5, 0.5});
        roadBarrier->setLocation({0.3, 0.0, -4});
        roadBarrier->initAsMeshNode(smd);

        auto roadBarrier2 = new SceneNode("roadBarrier2");
        roadBarrier2->setRotation({0, 28, 0});
        roadBarrier2->setScale({0.5, 0.5, 0.5});
        roadBarrier2->setLocation({1.6, 0.0, -25});
        roadBarrier2->initAsMeshNode(smd);

        auto shrub = new SceneNode("shrub");
        smd.mesh = game->getMeshByName("shrub");
        smd.texture = game->getTextureByName("shrub_albedo");
        smd.normalMap = game->getTextureByName("shrub_normal");
        smd.uvScale2 = {1, 1};
        smd.uvScale = 1;
        smd.uvPan = {0, 0};
        shrub->setRotation({0, 18, 0});
        shrub->setScale({1, 1, 1});
        shrub->setLocation({-0.8, 0.0, -2});
        shrub->initAsMeshNode(smd);

        auto hydrantdNode = new SceneNode("hydrant");
        smd.mesh = game->getMeshByName("fire_hydrant_1k");
        smd.texture = game->getTextureByName("hydrant_albedo");
        smd.normalMap = game->getTextureByName("hydrant_normal");
        smd.uvScale2 = {1, 1};
        smd.uvScale = 1;
        smd.uvPan = {0, 0};
        hydrantdNode->setScale({1.5, 1.5, 1.5});
        hydrantdNode->setLocation({0, 0.0, 0});
        hydrantdNode->initAsMeshNode(smd);

        auto wall1Node = std::make_unique<SceneNode>("wall_left");
        wall1Node->setOrientation(glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0)));
        smd.mesh = game->getMeshByName("round_wall");
        smd.texture = game->getTextureByName("ground_albedo");
        smd.normalMap = game->getTextureByName("ground_normal");
        smd.uvScale2 = {5, 5};
        smd.tint = {.5, 1.5, 1, 1};
        smd.uvScale = 1;
        smd.uvPan = {0, 0};
        wall1Node->setScale({2,2, 2});
        wall1Node->setLocation({-25, 0.0, -20});
        wall1Node->initAsMeshNode(smd);

        auto wall2Node = std::make_unique<SceneNode>("wall_right");
        wall2Node->setScale({2, 1, 10});
        wall2Node->setLocation({25, 0.0, 40});
        wall2Node->initAsMeshNode(smd);


        auto sunNode = std::make_unique<SceneNode>("sun");
        //sunNode->disable();
        auto sun = new Light();
        sun->type = LightType::Directional;
        sun->color = glm::vec4(.615, .615, .718, 1);
        sun->location = glm::vec3(6,5, 2.5);
        sun->lookAtTarget = glm::vec3(0, 0, 0);
        sun->shadowBias = 0.001;
        sun->calculateDirectionFromCurrentLocationLookat();
        sun->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
        sunNode->initAsLightNode(sun);


        auto pointLight1 = new Light();
        pointLight1->type = LightType::Point;
        pointLight1->color = glm::vec3(1.2, 1.1, 1.1);
        pointLight1->location = {4, 4,0};
        pointLight1->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
        auto plNode1 = std::make_unique<SceneNode>("pointLight1");
        plNode1->initAsLightNode(pointLight1);

        auto plNode2 = std::make_unique<SceneNode>("pointLight2");
        plNode2->setLocation({-13, 2,0});
        auto pointLight2 = new Light();
        pointLight2->type = LightType::Point;
        pointLight2->color = glm::vec3(.1, 0.0, .2);
        pointLight2->location = plNode2->getLocation();
        pointLight2->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});

        plNode2->initAsLightNode(pointLight2);

        cameraNode = std::make_unique<SceneNode>("camera");
        cameraNode->enable();
        cameraNode->initAsCameraNode(game->getGameplayCamera());
        auto cam = cameraNode->getCamera();
        //cam->updateLocation({0,34, 8});
        cameraNode->setLocation({0, 34, 8});
        cam->updateLookupTarget({0, 0, 1});
        cam->updateNearFar(0.5, 60);
        cam->addPostProcessEffect(new BloomEffect());
        cameraMover = new CameraMover(cameraNode->getCamera());


        auto heroNode = std::make_shared<SceneNode>("hero");
        heroNode->setLocation({0, 0.2, 0});
        MeshDrawData heroMeshData;
        heroMeshData.mesh = game->getMeshByName("hero_small");
        heroMeshData.texture = game->getTextureByName("hero_albedo.png");
        heroMeshData.normalMap = game->getTextureByName("hero_normal");
        heroMeshData.shader = new Shader();
        heroMeshData.shader->initFromFiles("../src/engine/editor/assets/shaders/colored_mesh.vert", "../src/engine/editor/assets/shaders/colored_mesh.frag");
        heroNode->initAsMeshNode(heroMeshData);

        auto shotCursorNode = std::make_shared<SceneNode>("shotCursor");
        shotCursorNode->setLocation({0, 0.2, 0});
        shotCursorNode->setScale({0.5, 1, 0.5});
        MeshDrawData cursorMeshData;
        cursorMeshData.mesh = game->getMeshByName("cursor_plane");
        cursorMeshData.color = {0.1,0.1, 1, 0.3};
        cursorMeshData.shader = heroMeshData.shader;
        cursorMeshData.castShadow = false;
        shotCursorNode->initAsMeshNode(cursorMeshData);

        padNode = std::make_shared<SceneNode>("heroPad");
        padNode->setLocation({0, 0.5, 5});
        padNode->addChild(heroNode);
        padNode->addChild(shotCursorNode);
        padNode->setScale({0.6, 0.6, 0.6});
        MeshDrawData padMeshData;
        padMeshData.mesh = game->getMeshByName("hero_pad");
        padMeshData.color = {0.2,0.2, 0, 1};
        padMeshData.shader = heroMeshData.shader;
        padNode->initAsMeshNode(padMeshData);

        // Enemies
        std::vector<std::unique_ptr<SceneNode>> enemyList;
        for (int i = 0; i < 3; i++) {
            auto enemy = std::make_unique<SceneNode>("spiderbot1_" + std::to_string(i));
            enemy->setLocation({-5 + (i*8), 0, -6});
            MeshDrawData targetDummyMeshData;
            targetDummyMeshData.shader = basicShader.get();
            targetDummyMeshData.mesh = game->getMeshByName("spiderbot4");
            targetDummyMeshData.texture = game->getTextureByName("spiderbot1_diffuse");
            // targetDummyMeshData.normalMap = game->getTextureByName("ground_normal");
            enemy->initAsMeshNode(targetDummyMeshData);
            {
                auto peSmoke = new gru::ParticleEmitter(nullptr, game->getTextureByName("smoke_diffuse"), gru::EmitterType::SMOKE, enemy->getLocation(), 200, true, false);
                auto peExplosion = new gru::ParticleEmitter(game->getMeshByName("cubby"), game->getTextureByName("smoke_diffuse"), gru::EmitterType::EXPLOSION, enemy->getLocation(), 100, true, false);
                gru::EmitterExecutionRule rule;
                rule.loop = false;
                rule.startDelay = .1;
                rule.maxDuration = 1.50;
                rule.locationOffset = enemy->getLocation();   // TODO actually apply this somewhere
                auto psystem = new gru::ParticleSystem();
                psystem->addEmitter(peSmoke, rule);
                rule.startDelay = 0.0;
                rule.maxDuration = .50;
                psystem->addEmitter(peExplosion, rule);
                auto particleSystemNode = new SceneNode("particleSystem_" + std::to_string(i));
                particleSystemNode->disable();
                particleSystemNode->initAsParticleSystemNode(psystem);
                // TODO could also be a child of the enemy?!
                enemy->setExtraData(particleSystemNode);

                // Set this system as a child of the respective enemy.
                //enemyExplosionParticles.push_back(particleSystemNode);
            }

            enemyList.push_back(std::move(enemy));
        }

        spawnDecalQuadMesh = createQuadMesh(PlanePivot::center);
        std::vector<std::shared_ptr<SceneNode>> spawnDecals;
        // Spawn decals
        for (int i = 0; i < 3; i++) {
            auto spawnDecal = std::make_shared<SceneNode>("spawnDecal_" + std::to_string(i));
            spawnDecal->setLocation({-18 + (i*8), 0.25, 5});
            spawnDecal->setOrientation(glm::angleAxis<float>(glm::radians(-90.0f), glm::vec3{1, 0, 0}));
            spawnDecal->setScale({4, 4, 1});
            MeshDrawData mdd;
            mdd.shader = basicShader.get();
            mdd.mesh = spawnDecalQuadMesh.get();
            mdd.castShadow = false;
            mdd.texture = game->getTextureByName("spawn_decal");
            mdd.onRender = [](MeshDrawData md) {
                // TODO Not sure about the "out of band" binding of the shader here.
                glUseProgram(md.shader->handle);
                // This changes the frame uniform which changes the color, making it pulse.
                // Turn off for now:
                //md.shader->setFloatValue(currentFrame, "frame");
            };
            spawnDecal->initAsMeshNode(mdd);
            spawnDecals.push_back(std::move(spawnDecal));
        }


        scene = new Scene();
        scene->setUICamera(game->getUICamera());
        scene->addNode(cameraNode);
        scene->addNode(std::move(terrainNode));
        // scene->addNode(std::move(plNode1));
        // scene->addNode(std::move(plNode2));
        scene->addNode(std::move(subTerrain));
        scene->addNode(std::move(grassNode1));
        scene->addNode(std::move(grassNode2));
        scene->addNode(std::move(stoneFieldNode1));
        scene->addNode(std::move(stoneFieldNode2));
        scene->addNode(std::move(sunNode));
        scene->addNode(padNode);
        for (auto& e : enemyList) {
            scene->addNode(std::move(e));
        }
        for (auto& sd : spawnDecals) {
            scene->addNode(std::move(sd));
        }

        // The explosion particle systems should be part of each enemy node, which will then
        // activate them on demand.
        // Better than this sidetracked list here.
        // for (auto psn : enemyExplosionParticles) {
        //     scene->addNode(psn);
        // }




        characterController = new CharacterController(scene->findNodeById("heroPad"));
        characterController->setMovementSpeed(10);
        characterController->setRotationSpeed(400);

        playerBulletPool = std::make_shared<PlayerBulletPool>(scene, game->getMeshByName("planar_bullet"), game->getTextureByName("planar_bullet_diffuse"), basicShader.get());
        playerShootingLogic = std::make_unique<PlayerShooting>(padNode, playerBulletPool);

        //peSmoke2 = new gru::ParticleEmitter(nullptr, game->getTextureByName("smoke_diffuse"), gru::EmitterType::SMOKE, {-3, 0.5, 7}, 200);



    }
} // ttg