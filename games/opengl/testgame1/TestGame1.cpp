//
// Created by mgrus on 13.02.2025.
//

#include "TestGame1.h"


DefaultApp* getGame() {
    return new TestGame1();
}

void TestGame1::init() {
    DefaultApp::init();
    mechShader = new Shader();
    mechShader->initFromFiles("../games/opengl/testgame1/assets/shaders/mech.vert", "../games/opengl/testgame1/assets/shaders/mech.frag");
    auto cam = getGameplayCamera();
    cam->updateLocation({0, 2.8, 10});
    cam->updateLookupTarget({0, 1, -2});
    cameraMover = new CameraMover(cam);

    scene = new Scene();
    auto mechNode = new SceneNode();
    mechNode->location = glm::vec3(0, 0, -5);
    mechNode->mesh = getMeshByName("mech");
    mechNode->texture = getTextureByName("mech_albedo");
    mechNode->normalMap = getTextureByName("mech_normal");
    mechNode->type = SceneNodeType::Mesh;
    mechNode->uvScale= 1;
    mechNode->shader = mechShader;
    scene->addNode(mechNode);

    auto playerNode = new SceneNode();
    playerNode->location = glm::vec3(-2, 0, 2);
    playerNode->mesh = getMeshByName("human4");
    playerNode->texture = getTextureByName("debug_texture");
    //playerNode->normalMap = getTextureByName("debug_normal");
    playerNode->shader = mechShader;
    playerNode->rotation = glm::vec3(0, 180, 0);
    playerNode->type = SceneNodeType::Mesh;
    scene->addNode(playerNode);

    auto groundNode = new SceneNode();
    groundNode->location = glm::vec3(0, 0, 0);
    groundNode->mesh = getMeshByName("ground_plane");
    groundNode->texture = getTextureByName("ground_albedo");
    groundNode->normalMap = getTextureByName("debug_normal");
    groundNode->type = SceneNodeType::Mesh;
    groundNode->uvScale = 205;
    groundNode->scale = glm::vec3(20, 0.5, 20);
    groundNode->shader = mechShader;
    scene->addNode(groundNode);

    sun = new Light();
    sun->location = {3, 3,8 };
    sun->lookAtTarget = {0,0, 0};
    sun->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
    scene->setDirectionalLight(sun);
    scene->setCamera(cam);

}

void TestGame1::update() {
    cameraMover->update();
}

void TestGame1::render() {

    scene->render();

    // MeshDrawData dd;
    // dd.mesh = getMeshByName("mech");
    // dd.location = {0, 0, -8};
    // dd.texture = getTextureByName("mech_albedo");
    // dd.normalMap = getTextureByName("mech_normal");
    // dd.uvScale = 1;
    // dd.directionalLight = sun;
    // dd.camera = getGameplayCamera();
    // dd.shader = mechShader;
    // drawMesh(dd);
    //
    // dd.location = {-3, 0, -4};
    // dd.rotationEulers = {0, 180, 0};
    // drawMesh(dd);
    //
    // dd.location = {4, 0, -7.3};
    // drawMesh(dd);
    //
    //
    // dd.mesh = getMeshByName("ground_plane");
    // dd.texture = getTextureByName("ground_albedo");
    // dd.normalMap = getTextureByName("debug_normal");
    // dd.scale = {5, 1, 5};
    // dd.uvScale = 45;
    // drawMesh(dd);
    renderFPS();
}

bool TestGame1::shouldStillRun() {
    return true;
}

bool TestGame1::shouldAutoImportAssets() {
    return true;
}

std::vector<std::string> TestGame1::getAssetFolder() {
    return {"../games/opengl/testgame1/assets"};
}
