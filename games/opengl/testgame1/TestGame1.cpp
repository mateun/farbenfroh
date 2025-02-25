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

    scene = new Scene();


    auto cam = getGameplayCamera();
    cam->updateLocation({0, 1.8, 10});
    cam->updateLookupTarget({0, 1, -2});
    cam->setInitialForward(camFwd(cam));
    cameraMover = new CameraMover(cam);
    auto mainCamNode = new SceneNode();
    mainCamNode->initAsCameraNode(cam);
    scene->addNode(mainCamNode);


    auto mechNode = new SceneNode("mech");
    SceneMeshData smd;
    smd.mesh =  getMeshByName("mech");
    smd.texture = getTextureByName("mech_albedo");
    smd.normalMap = getTextureByName("mech_normal");
    smd.shader = mechShader;
    mechNode->initAsMeshNode(&smd);
    mechNode->setLocation(glm::vec3(0, 0, -5));
    scene->addNode(mechNode);

    playerNode = new SceneNode("player");
    //playerNode->disable();
    smd.mesh = getMeshByName("human4_oriented");
    smd.texture = getTextureByName("debug_texture");
    smd.normalMap = getTextureByName("mech_normal");
    smd.shader = skinnedShader;
    smd.skinnedMesh = true;
    playerNode->setLocation({-2, 0, 2});
    playerNode->initAsMeshNode(&smd);
    scene->addNode(playerNode);

    auto groundNode = new SceneNode("ground");
    //groundNode->disable();
    groundNode->setLocation (glm::vec3(0, 0, 0));
    groundNode->setScale(glm::vec3(20, 0.5, 20));
    smd.mesh = getMeshByName("ground_plane");
    smd.texture = getTextureByName("ground_albedo");
    smd.normalMap = getTextureByName("debug_normal");
    smd.uvScale = 205;
    smd.shader = mechShader;
    smd.skinnedMesh = false;
    groundNode->initAsMeshNode(&smd);
    scene->addNode(groundNode);

    // TODO also put lights back into more generic node mode.
    sun = new Light();
    sun->type = LightType::Directional;
    sun->color = glm::vec3(0.5, .1, .1);
    sun->location = {3, 3,8 };
    sun->lookAtTarget = {0,0, 0};
    sun->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
    auto sunNode = new SceneNode();
    sunNode->initAsLightNode(sun);
    scene->addNode(sunNode);

    auto sun2 = new Light();
    sun2->type = LightType::Directional;
    sun2->color = glm::vec3(0.1, .1, .9);
    sun2->location = {-3, 2,6 };
    sun2->lookAtTarget = {0,0, 0};
    sun2->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
    auto sunNode2 = new SceneNode();
    sunNode2->initAsLightNode(sun2);
    scene->addNode(sunNode2);

    // Some pointlights
    auto pointLight1 = new Light();
    pointLight1->type = LightType::Point;
    pointLight1->color = glm::vec3(0.1, 0.1, -2);
    pointLight1->location = {0, 4.5,0};
    pointLight1->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
    auto plNode1 = new SceneNode();
    plNode1->initAsLightNode(pointLight1);
    scene->addNode(plNode1);

    auto pointLight2 = new Light();
    pointLight2->type = LightType::Point;
    pointLight2->color = glm::vec3(0.9, 0.9, 0);
    pointLight2->location = {-11.5, 4.5,1};
    pointLight2->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
    auto plNode2 = new SceneNode();
    plNode2->initAsLightNode(pointLight2);
    scene->addNode(plNode2);

    cameraCinematic = new Cinematic(scene, 10);
    auto camTrack = cameraCinematic->addTrack(mainCamNode, "mainCamTrack");
    camTrack->addKeyFrame(ChannelType::Location, 0, {0, 1.8, 10});
    camTrack->addKeyFrame(ChannelType::Location, 5, {-8, 0.8, -2});
    camTrack->addKeyFrame(ChannelType::Location, 10, {-8, 0.8, -2});
    // TODO fix rotation application
    camTrack->addKeyFrame(ChannelType::Rotation, 0, {0, 0, 0}, AngleUnit::RAD);
    camTrack->addKeyFrame(ChannelType::Rotation, 7, {0, -1.83f, 0}, AngleUnit::RAD);
    camTrack->addKeyFrame(ChannelType::Rotation, 8, {0, -1.83f, 0}, AngleUnit::RAD);
    camTrack->addKeyFrame(ChannelType::Rotation, 10, {0.2, -1.83f, 0}, AngleUnit::RAD);

    mechFlyCinematic = new Cinematic(scene, 10);
    auto mechTrack = mechFlyCinematic->addTrack(mechNode, "mechTrack");
    mechTrack->addKeyFrame(ChannelType::Location, 0, {0, 0, -5});
    mechTrack->addKeyFrame(ChannelType::Location, 7, {0, 4, -5});
    mechTrack->addKeyFrame(ChannelType::Location, 10, {0, 10, -8});
    mechTrack->addKeyFrame(ChannelType::Rotation, 0, {0, 0, 0}, AngleUnit::DEGREES);
    mechTrack->addKeyFrame(ChannelType::Rotation, 4, {0, 45, 0}, AngleUnit::DEGREES);
    mechTrack->addKeyFrame(ChannelType::Rotation, 10, {0, -60, 0}, AngleUnit::DEGREES);


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

    // Check for cinematic activation:
    {
        if (keyPressed('C')) {
            if (!cameraCinematic->isActive()) {
                cameraCinematic->play();
            }
        }
        if (cameraCinematic->isActive()) {
            cameraCinematic->update();
            return;
        }

        if (keyPressed('M')) {
            if (!mechFlyCinematic->isActive()) {
                mechFlyCinematic->play();
            }
        }
        if (mechFlyCinematic->isActive()) {
            mechFlyCinematic->update();
            return;
        }
    }

    updateSwitcher->update();

    // Temp. implement animation switching here
    //idlePlayer->update();
    //playerNode->boneMatrices = idlePlayer->getCurrentBoneMatrices();
    animationController->update();
    playerNode->updateBoneMatrices(animationController->getBoneMatrices());

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

    // TODO check for any cinematics to render? Not sure, if this is really necessary..
    // the cinematics apply all changes on the scene diretly anyway (which might be good or not in its own right..)

    scene->render();

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
