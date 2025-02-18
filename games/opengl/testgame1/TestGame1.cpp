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
    skinnedShader = new Shader();
    skinnedShader->initFromFiles("../games/opengl/testgame1/assets/shaders/skinned.vert", "../games/opengl/testgame1/assets/shaders/mech.frag");

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

    playerNode = new SceneNode();
    playerNode->location = glm::vec3(-2, 0, 2);
    playerNode->mesh = getMeshByName("human4_oriented");
    playerNode->texture = getTextureByName("debug_texture");
    playerNode->normalMap = getTextureByName("mech_normal");
    playerNode->shader = skinnedShader;
    playerNode->skinnedMesh = true;
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

    characterController = new CharacterController(playerNode);
    updateSwitcher = new UpdateSwitcher({characterController, cameraMover}, VK_F10);

    idlePlayer = new AnimationPlayer(getMeshByName("human4_oriented")->findAnimation("idle"), getMeshByName("human4_oriented"));
    walkPlayer = new AnimationPlayer(getMeshByName("human4_oriented")->findAnimation("walk"), getMeshByName("human4_oriented"));
    idlePlayer->play(true);

    animationController = new AnimationController(getMeshByName("human4_oriented"));
    auto idleState = new AnimationState(getMeshByName("human4_oriented")->findAnimation("idle"), "idle");
    auto walkState = new AnimationState(getMeshByName("human4_oriented")->findAnimation("walk"), "walk");
    animationController->addAnimationState(idleState);
    animationController->addAnimationState(walkState);
    auto idleWalkTrans = new AnimationTransition(animationController, idleState, walkState);
    auto idleWalkConstraint = new TransitionConstraint();
    idleWalkConstraint->propertyKey = "movementSpeed";
    idleWalkConstraint->property.floatValue = 0.5;
    idleWalkConstraint->property.propertyType = PropertyType::FLOAT;
    idleWalkConstraint->constraintOperator = ConstraintOperator::GREATEREQUAL;
    idleWalkTrans->addConstraint(idleWalkConstraint);

}

void TestGame1::update() {
    updateSwitcher->update();

    // Temp. implement animation switching here
    //idlePlayer->update();
    //playerNode->boneMatrices = idlePlayer->getCurrentBoneMatrices();
    animationController->update();
    playerNode->boneMatrices = animationController->getBoneMatrices();

    // A little test setup:
    // We wait for 2 seconds, then we set the animation controller property
    // to 1.0, expecting the transition from idle to walk to trigger.
    // (which is set to trigger when the movmentSpeed is >= 0.5)
    static float timePassed = 0;
    timePassed += ftSeconds;
    if (timePassed > 2.0) {
        AnimationProperty prop;
        prop.propertyType = PropertyType::FLOAT;
        prop.floatValue = 1.0f;
        animationController->setProperty("movementSpeed", prop);
    }
    // end temp


}

void TestGame1::render() {

    scene->render();

    // MeshDrawData dd;
    // dd.mesh = getMeshByName("human4_oriented");
    // dd.location = {-1, 1, -4};
    // dd.texture = getTextureByName("mech_albedo");
    // dd.normalMap = getTextureByName("mech_normal");
    // dd.uvScale = 1;
    // dd.directionalLight = sun;
    // dd.camera = getGameplayCamera();
    // dd.shader = skinnedShader;
    // dd.skinnedDraw = true;
    // dd.boneMatrices = idlePlayer->getCurrentBoneMatrices();
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
    // dd.skinnedDraw = false;
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
