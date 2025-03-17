//
// Created by mgrus on 02.03.2025.
//

#include "GameplayLevel.h"

#include "EnemyExplosionComponent.h"
#include  "../../../src/engine/game/default_game.h"
class gru::ParticleSystem;
class gru::ParticleEmitter;

namespace ttg {
    GameplayLevel::GameplayLevel(DefaultGame *game, const std::string& name): GameLevel(game, name) {
    }

    void GameplayLevel::renderShadowBias() {
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
        int activeBullets = 0;
        for (auto b : playerBulletPool) {
            if (b->isActive()) {
                activeBullets++;
            }
        }
        char buf[175];
        sprintf_s(buf, 160, "# player bullets:%03d",
                  activeBullets);
        flipUvs(false);
        font->renderText(buf, {2, scaled_height - 40, 0.88});

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

        // Active enemy explosions
        {
            for (auto ee: activeExplosions) {
                ee->render(game->getGameplayCamera());
            }
        }

        game->renderFPS();
        renderShadowBias();
        renderPlayerStats();
    }

    SceneNode * GameplayLevel::findFirstInactive(const std::vector<SceneNode *> &nodeList) {
        for (auto n : nodeList) {
            if (!n->isActive()) {
                return n;
            }
        }

        return nullptr;
    }

    void GameplayLevel::updateActiveEnemyExplosions() {
        for (auto ee: activeExplosions) {
            ee->update();
        }
    }

    void GameplayLevel::updatePlayerBullets() {
        for (auto b : playerBulletPool) {
            if (!b->isActive()) {
                continue;
            }

            auto pos = b->getLocation();
            pos += b->getForwardVector() * ftSeconds * 45.0f;
            b->setLocation(pos);
            BulletData* bd = (BulletData*) b->getExtraData();
            bd->currentLifeInSeconds += ftSeconds;
            if (bd->currentLifeInSeconds >= bd->maxLifeInSeconds) {
                b->disable();
                bd->currentLifeInSeconds = 0;
            }

            // Check for collision with enemies
            for (auto e: enemyList) {
                if (!e->isActive()) {
                    continue;
                }
                float distance = glm::distance(pos, e->getLocation());
                if (distance < 2) {
                    b->disable();
                    e->disable();
                    auto explosionComp = (gru::ParticleSystem*) e->getExtraData();
                    activeExplosions.push_back(explosionComp);
                }
            }

            // TODO check wall collision
        }
    }

    void GameplayLevel::checkPlayerCollision() {
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
        scene->update();
        if (keyPressed(VK_F11)) {
            inFlyCamDebugMode = !inFlyCamDebugMode;
            scene->activateDebugFlyCam(inFlyCamDebugMode);
        }

        if (!inFlyCamDebugMode) {
            //cameraMover->update();
            characterController->update();

            checkPlayerCollision();
            cameraUpdate();


            // Player shooting
            // TODO move into own "controller" class?

            static float lastShotInterval = 0.0f;
            lastShotInterval += ftSeconds;
            if (getControllerAxis(ControllerAxis::R_TRIGGER, 0) > 0.1 && lastShotInterval >= 0.1f) {
                lastShotInterval = 0.0f;
                auto bulletNode = findFirstInactive(playerBulletPool);
                if (!bulletNode) {
                    throw std::runtime_error("No bullet node found!!");
                }
                bulletNode->enable();
                bulletNode->setLocation(padNode->getLocation() + glm::vec3{0, 1.0f, 0} + padNode->getForwardVector() * .5f);
                bulletNode->setOrientation(padNode->getWorldOrientation());
                BulletData* bd = (BulletData*)bulletNode->getExtraData();
                bd->currentLifeInSeconds = 0;
            }

            updatePlayerBullets();
            updateActiveEnemyExplosions();
            //psystem0->update();
            //peSmoke0->update();
            //peSmoke1->update();
            //peSmoke2->update();
        }

    }

    void GameplayLevel::init() {

        basicShader = new Shader();
        basicShader->initFromFiles("../games/opengl/test_terrain/assets/shaders/basic.vert", "../games/opengl/test_terrain/assets/shaders/basic.frag");

        terrain = new Terrain(50, 50);

        MeshDrawData smd;

        terrainNode = new SceneNode("terrain");
        //terrainNode->disable();
        smd.mesh = terrain->getMesh();
        terrainNode->setScale({1, 1, 20});
        smd.shader = basicShader;
        smd.texture = game->getTextureByName("ground_albedo");
        smd.normalMap = game->getTextureByName("ground_normal");
        smd.uvScale2 = {100, 2000};
        smd.uvScale = 1;
        terrainNode->initAsMeshNode(smd);

        auto roadNode = new SceneNode("road");
        smd.mesh = game->getMeshByName("road_plane");
        smd.texture = game->getTextureByName("asphalt_albedo");
        smd.normalMap = game->getTextureByName("asphalt_normal");
        smd.uvScale2 = {1, 5};
        smd.uvScale = 1;
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

        auto wall1Node = new SceneNode("wall_left");
        smd.mesh = game->getMeshByName("wall");
        smd.texture = game->getTextureByName("color_grid");
        smd.normalMap = nullptr;
        smd.uvScale2 = {1, 10};
        smd.uvScale = 1;
        smd.uvPan = {0, 0};
        wall1Node->setScale({2, 1, 10});
        wall1Node->setLocation({-25, 0.0, 40});
        wall1Node->initAsMeshNode(smd);

        auto wall2Node = new SceneNode("wall_right");
        wall2Node->setScale({2, 1, 10});
        wall2Node->setLocation({25, 0.0, 40});
        wall2Node->initAsMeshNode(smd);


        auto sunNode = new SceneNode("sun");
        auto sun = new Light();
        sun->type = LightType::Directional;
        sun->color = glm::vec4(.9, .9, .9, 1);
        sun->location = glm::vec3(4,5, 2);
        sun->lookAtTarget = glm::vec3(0, 0, 0);
        sun->shadowBias = 0.001;
        sun->calculateDirectionFromCurrentLocationLookat();
        sun->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
        sunNode->initAsLightNode(sun);

        // auto pointLight1 = new Light();
        // pointLight1->type = LightType::Point;
        // pointLight1->color = glm::vec3(0.8, 0.1, 0.1);
        // pointLight1->location = {4, 4,0};
        // pointLight1->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
        // auto plNode1 = new SceneNode();
        // plNode1->initAsLightNode(pointLight1);
        //
        // auto pointLight2 = new Light();
        // pointLight2->type = LightType::Point;
        // pointLight2->color = glm::vec3(0.0, 0.0, 1);
        // pointLight2->location = {-3, 4,0};
        // pointLight2->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
        // auto plNode2 = new SceneNode();
        // plNode2->initAsLightNode(pointLight2);

        cameraNode = new SceneNode("camera");
        cameraNode->enable();
        cameraNode->initAsCameraNode(game->getGameplayCamera());
        auto cam = cameraNode->getCamera();
        //cam->updateLocation({0,34, 8});
        cameraNode->setLocation({0, 34, 8});
        cam->updateLookupTarget({0, 0, 1});
        cam->updateNearFar(0.5, 60);
        cameraMover = new CameraMover(cameraNode->getCamera());

        heroNode = new SceneNode("hero");
        heroNode->setLocation({0, 0.2, 0});
        MeshDrawData heroMeshData;
        heroMeshData.mesh = game->getMeshByName("hero_small");
        heroMeshData.texture = game->getTextureByName("hero_albedo.png");
        heroMeshData.normalMap = game->getTextureByName("hero_normal");
        heroMeshData.shader = new Shader();
        heroMeshData.shader->initFromFiles("../src/engine/editor/assets/shaders/colored_mesh.vert", "../src/engine/editor/assets/shaders/colored_mesh.frag");
        heroNode->initAsMeshNode(heroMeshData);

        shotCursorNode = new SceneNode("shotCursor");
        shotCursorNode->setLocation({0, 0.2, 0});
        shotCursorNode->setScale({0.5, 1, 0.5});
        MeshDrawData cursorMeshData;
        cursorMeshData.mesh = game->getMeshByName("cursor_plane");
        cursorMeshData.color = {0.1,0.1, 1, 0.3};
        cursorMeshData.shader = heroMeshData.shader;
        cursorMeshData.castShadow = false;
        shotCursorNode->initAsMeshNode(cursorMeshData);

        padNode = new SceneNode("heroPad");
        padNode->setLocation({0, 0.5, 5});
        padNode->addChild(heroNode);
        padNode->addChild(shotCursorNode);
        padNode->setScale({0.6, 0.6, 0.6});
        MeshDrawData padMeshData;
        padMeshData.mesh = game->getMeshByName("hero_pad");
        padMeshData.color = {0.2,0.2, 0, 1};
        padMeshData.shader = heroMeshData.shader;
        padNode->initAsMeshNode(padMeshData);

        // Prepare emitters for enemy explosions:


        // Enemies
        for (int i = 0; i < 25; i++) {
            auto targetDummy = new SceneNode("targetDummy_" + std::to_string(i));
            targetDummy->setLocation({-25 + (i*5), 0, -6});
            MeshDrawData targetDummyMeshData;
            targetDummyMeshData.shader = basicShader;
            targetDummyMeshData.mesh = game->getMeshByName("target_dummy");
            targetDummyMeshData.texture = game->getTextureByName("dummy-target-diffuse");
            targetDummy->initAsMeshNode(targetDummyMeshData);
            {
                auto peSmoke = new gru::ParticleEmitter(nullptr, game->getTextureByName("smoke_diffuse"), gru::EmitterType::SMOKE, targetDummy->getLocation(), 200, true, false);
                auto peExplosion = new gru::ParticleEmitter(game->getMeshByName("cubby"), game->getTextureByName("color_grid"), gru::EmitterType::EXPLOSION, targetDummy->getLocation(), 30, true, false);
                gru::EmitterExecutionRule rule;
                rule.loop = false;
                rule.startDelay = .1;
                rule.maxDuration = 1.50;
                rule.locationOffset = targetDummy->getLocation();   // TODO actually apply this somewhere
                auto psystem = new gru::ParticleSystem();
                psystem->addEmitter(peSmoke, rule);
                rule.startDelay = 0.0;
                rule.maxDuration = .50;
                psystem->addEmitter(peExplosion, rule);
                targetDummy->setExtraData(psystem);
            }

            enemyList.push_back(targetDummy);
        }


        scene = new Scene();
        scene->addNode(cameraNode);
        scene->addNode(terrainNode);
        scene->addNode(wall1Node);
        scene->addNode(wall2Node);
        //scene->addNode(hydrantdNode);
        //scene->addNode(roadNode);
        //scene->addNode(roadNode2);
        //scene->addNode(roadBarrier);
        //scene->addNode(roadBarrier2);
        //scene->addNode(shrub);
        scene->addNode(sunNode);
        scene->addNode(padNode);
        for (auto e : enemyList) {
            scene->addNode(e);
        }


        characterController = new CharacterController(padNode);
        characterController->setMovementSpeed(10);
        characterController->setRotationSpeed(400);

        // Player bullet pool
        // A list of bullets which all start in active but are activated at shooting.
        // When being too old, they get reset again, a bit like particles.

        playerBulletMeshData = new MeshDrawData();
        playerBulletMeshData->texture = game->getTextureByName("planar_bullet_diffuse");
        playerBulletMeshData->mesh = game->getMeshByName("planar_bullet");
        playerBulletMeshData->shader = basicShader;
        playerBulletMeshData->castShadow = false;
        playerBulletMeshData->subroutineFragBind = "calculateSingleColor";
        //playerBulletMeshData->lit = false;
        for (int i = 0; i< 50; i++) {
            auto bulletNode = new SceneNode("playerBullet_" + std::to_string(i));
            bulletNode->disable();
            bulletNode->initAsMeshNode(*playerBulletMeshData);
            bulletNode->setScale({1, 1, 1});
            bulletNode->setExtraData(new BulletData());

            playerBulletPool.push_back(bulletNode);
            scene->addNode(bulletNode);
        }

        //peSmoke2 = new gru::ParticleEmitter(nullptr, game->getTextureByName("smoke_diffuse"), gru::EmitterType::SMOKE, {-3, 0.5, 7}, 200);



    }
} // ttg