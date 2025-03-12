//
// Created by mgrus on 02.03.2025.
//

#include "GameplayLevel.h"
#include  "../../../src/engine/game/default_game.h"

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
        font->renderText(buf, {scaled_width - 200, -16, 0.9});
    }

    void GameplayLevel::render() {
        if (keyPressed('B')) {
            scene->getLightsOfType(LightType::Directional)[0]->shadowBias += 0.0001f;
        }
        if (keyPressed('V')) {
            scene->getLightsOfType(LightType::Directional)[0]->shadowBias -= 0.0001f;
        }
        scene->render();
        game->renderFPS();
        renderShadowBias();
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
        }

    }

    void GameplayLevel::init() {

        basicShader = new Shader();
        basicShader->initFromFiles("../games/opengl/test_terrain/assets/shaders/basic.vert", "../games/opengl/test_terrain/assets/shaders/basic.frag");

        terrain = new Terrain(50, 50);

        SceneMeshData smd;

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

        auto sunNode = new SceneNode("sun");
        auto sun = new Light();
        sun->type = LightType::Directional;
        sun->color = glm::vec4(1, 1, 1, 1);
        sun->location = glm::vec3(5, 5, 0);
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
        cam->updateLocation({0,14, 8});
        cam->updateLookupTarget({0, 0, 1});
        cam->updateNearFar(0.5, 30);
        cameraMover = new CameraMover(cameraNode->getCamera());

        heroNode = new SceneNode("hero");
        heroNode->setLocation({0, 0.2, 0});
        SceneMeshData heroMeshData;
        heroMeshData.mesh = game->getMeshByName("hero_small");
        heroMeshData.texture = game->getTextureByName("hero_albedo.png");
        heroMeshData.normalMap = game->getTextureByName("hero_normal");
        heroMeshData.shader = new Shader();
        heroMeshData.shader->initFromFiles("../src/engine/editor/assets/shaders/colored_mesh.vert", "../src/engine/editor/assets/shaders/colored_mesh.frag");
        heroNode->initAsMeshNode(heroMeshData);

        shotCursorNode = new SceneNode("shotCursor");
        shotCursorNode->setLocation({0, 1, 0});
        SceneMeshData cursorMeshData;
        cursorMeshData.mesh = game->getMeshByName("shotcursor");
        cursorMeshData.color = {1,1, 1, 0.5};
        cursorMeshData.shader = heroMeshData.shader;
        cursorMeshData.castShadow = false;
        shotCursorNode->initAsMeshNode(cursorMeshData);

        auto padNode = new SceneNode("heroPad");
        padNode->setLocation({0, 0.5, 5});
        padNode->addChild(heroNode);
        padNode->addChild(shotCursorNode);
        SceneMeshData padMeshData;
        padMeshData.mesh = game->getMeshByName("hero_pad");
        padMeshData.color = {0.2,0.2, 0, 1};
        padMeshData.shader = heroMeshData.shader;
        padNode->initAsMeshNode(padMeshData);

        scene = new Scene();
        scene->addNode(cameraNode);
        scene->addNode(terrainNode);
        scene->addNode(hydrantdNode);
        scene->addNode(roadNode);
        scene->addNode(roadNode2);
        scene->addNode(roadBarrier);
        scene->addNode(roadBarrier2);
        scene->addNode(shrub);
        scene->addNode(sunNode);
        scene->addNode(padNode);
        //scene->addNode(plNode1);
        //scene->addNode(plNode2);

        characterController = new CharacterController(padNode);

    }
} // ttg