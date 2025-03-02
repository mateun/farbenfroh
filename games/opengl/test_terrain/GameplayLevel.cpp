//
// Created by mgrus on 02.03.2025.
//

#include "GameplayLevel.h"
#include  "../../../src/engine/game/default_game.h"

namespace ttg {
    GameplayLevel::GameplayLevel(DefaultGame *game): GameLevel(game) {
    }

    void GameplayLevel::render() {

        scene->render();
        game->renderFPS();
    }

    void GameplayLevel::update() {
        cameraMover->update();

    }

    void GameplayLevel::init() {

        basicShader = new Shader();
        basicShader->initFromFiles("../games/opengl/test_terrain/assets/shaders/basic.vert", "../games/opengl/test_terrain/assets/shaders/basic.frag");

        terrain = new Terrain(50, 50);

        SceneMeshData smd;

        terrainNode = new SceneNode("terrain");
        smd.mesh = terrain->getMesh();
        smd.shader = basicShader;
        smd.texture = game->getTextureByName("ground_albedo");
        smd.normalMap = game->getTextureByName("ground_normal");
        smd.uvScale2 = {100, 100};
        smd.uvScale = 1;
        terrainNode->initAsMeshNode(&smd);

        auto roadNode = new SceneNode("road");
        smd.mesh = game->getMeshByName("road_plane");
        smd.texture = game->getTextureByName("asphalt_albedo");
        smd.normalMap = game->getTextureByName("asphalt_normal");
        smd.uvScale2 = {1, 5};
        smd.uvScale = 1;
        roadNode->setScale({1, 1, 5});
        roadNode->setLocation({0, 0.001, -1});
        roadNode->initAsMeshNode(&smd);

        auto sunNode = new SceneNode("sun");
        auto sun = new Light();
        sun->type = LightType::Directional;
        sun->color = glm::vec4(1, 1, 1, 1);
        sun->location = glm::vec3(-3, 10, 5);
        sun->lookAtTarget = glm::vec3(0, 0, 0);
        sun->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
        sunNode->initAsLightNode(sun);

        auto pointLight1 = new Light();
        pointLight1->type = LightType::Point;
        pointLight1->color = glm::vec3(0.8, 0.1, 0.1);
        pointLight1->location = {4, 4,0};
        pointLight1->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
        auto plNode1 = new SceneNode();
        plNode1->initAsLightNode(pointLight1);

        auto pointLight2 = new Light();
        pointLight2->type = LightType::Point;
        pointLight2->color = glm::vec3(0.0, 0.0, 1);
        pointLight2->location = {-3, 4,0};
        pointLight2->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
        auto plNode2 = new SceneNode();
        plNode2->initAsLightNode(pointLight2);


        auto cameraNode = new SceneNode("camera");
        cameraNode->initAsCameraNode(game->getGameplayCamera());
        cameraMover = new CameraMover(cameraNode->getCamera());

        scene = new Scene();
        scene->addNode(cameraNode);
        scene->addNode(terrainNode);
        scene->addNode(roadNode);
        scene->addNode(sunNode);
        //scene->addNode(plNode1);
        //scene->addNode(plNode2);

    }
} // ttg