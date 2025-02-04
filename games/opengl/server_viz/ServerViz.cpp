//
// Created by mgrus on 03.02.2025.
//

#include "ServerViz.h"



DefaultGame* getGame() {
    return new ServerViz();
}

void ServerViz::init() {
    DefaultGame::init();
    cameraMover = new CameraMover(getGameplayCamera());
    _fullscreenFrameBuffer = createFrameBuffer( 1024.0 * (1280.0/720.0) , 768);
    _postProcessedFrameBuffer = createFrameBuffer( 1024, 768);

    auto vertSource = readFile("../games/opengl/server_viz/assets/shaders/post_process.vert");
    auto fragSource = readFile("../games/opengl/server_viz/assets/shaders/post_process.frag");
    _postProcessShader = new Shader();
    createShader(vertSource, fragSource, _postProcessShader);

    _cameraIn3DWorld = new Camera();
    _cameraIn3DWorld = new Camera();
    _cameraIn3DWorld->location = {0, 1.8, 1};
    _cameraIn3DWorld->lookAtTarget = {0, 1.5, -5};
    _cameraIn3DWorld->type = CameraType::Perspective;

}

void ServerViz::update() {
    cameraMover->update();

    if (keyPressed('F')) {
        fade = !fade;
    }
}

void ServerViz::render() {

    bindCamera(getGameplayCamera());
    lightingOn();

    // Prepare framebuffer for fullscreen post-processing
    {
        activateFrameBuffer(_fullscreenFrameBuffer);
        glViewport(0, 0, _fullscreenFrameBuffer->texture->bitmap->width,
                    _fullscreenFrameBuffer->texture->bitmap->height);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }


    scale({1, 1,1});
    foregroundColor({0.6, 0.5, 0.6, .2});
    location(glm::vec3{0, 0, 0});
    gridLines(100);
    drawGrid();

    // Our ground plane
    uvScale(15);
    bindMesh(getMeshByName("cube"));
    bindTexture(getTextureByName("groundplane"));
    location(glm::vec3{0, 0, 0});
    scale({100, .1, 100});
    drawMesh();


    // // Ship
    uvScale(1);
    scale({1,1,1});
    flipUvs(true);
    bindMesh(getMeshByName("cube"));
    bindTexture(getTextureByName("cube_diffuse"));
    location(glm::vec3{0, 1, -2});
    drawMesh();
    location({0, 0, 0});

    // Deactivate the full screen framebuffer and render the texture as
    // a quad into the postprocessed framebuffer.
    // This will be the basis of what we display in our 3d world.
    {
        activateFrameBuffer(_postProcessedFrameBuffer);
        glViewport(0, 0, _postProcessedFrameBuffer->texture->bitmap->width,
                       _postProcessedFrameBuffer->texture->bitmap->height);
        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    lightingOff();
    bindCamera(getUICamera());
    bindTexture(_fullscreenFrameBuffer->texture);
    location({1024/2, 768/2, -0.1});
    static float rot = 0;
    if (fade) {
        //rot += 1000 * ftSeconds;
    } else {
        rot = 0;
    }
    rotation({0, 0, 90});
    static float scaleFactor = 1.0f;
    if (fade) {
        //scaleFactor += 30 * ftSeconds;
    } else {
        scaleFactor = 1;
    }
    static float tinto = 1.0f;
    if (fade) {
        tinto -= 1 * ftSeconds;
        tint({1, 1, 1, tinto});
    }
    scale({1024/scaleFactor, 768/scaleFactor, 1});
    forceShader(_postProcessShader);
    setUniformFloat(10, ftSeconds * 4000, _postProcessShader);
    scaled_width = 1024;
    scaled_height = 768;
    drawPlane();
    rotation({0, 0, 0});
    scale({1, 1, 1});
    tint({1, 1, 1, 1});
    forceShader(nullptr);

    // Now back to the main backbuffer.
    // We take the last framebuffer, which has the final image of the
    // inner arcade game. We use this as a texture to project onto our cabinet screen.
    activateFrameBuffer(nullptr);
    glViewport(0, 0, window_width, window_height);

    // Draw our arcade cabinet.
    // For now just the screen itself.

    bindCamera(_cameraIn3DWorld);

    bindMesh(getMeshByName("screen_plane"));
    foregroundColor({.1, 1, .1, .6});
    bindTexture(nullptr);
    bindTexture(_postProcessedFrameBuffer->texture);
    flipUvs(false);
    location({0, 1.8, -0.1});
    rotation({0, 0, 180});
    drawMesh();

    lightingOn();
    bindMesh(getMeshByName("cabinet"));
    bindTexture(nullptr);
    foregroundColor({1, 1, 1, 1});
    location({0, 1.8, -0.1});
    rotation({0, 0, 0});
    drawMesh();




    foregroundColor({0.9, 0.2, 0.2, .5});
    renderFPS();

}

bool ServerViz::shouldStillRun() {
    return true;
}

bool ServerViz::shouldAutoImportAssets() {
    return true;
}

std::vector<std::string> ServerViz::getAssetFolder() {
    return { "../games/opengl/server_viz/assets", "../assets/sound"};
}
