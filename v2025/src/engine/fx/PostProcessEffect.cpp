//
// Created by mgrus on 26.03.2025.
//

#include "PostProcessEffect.h"
#include <engine/graphics/PlanePivot.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/Geometry.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Application.h>
#include <engine/graphics/StatefulRenderer.h>

PostProcessEffect::PostProcessEffect() {
    quadMesh = gru::Geometry::createQuadMesh(PlanePivot::center);
}

GammaCorrectionEffect::GammaCorrectionEffect(): PostProcessEffect() {
    gammaCorrectionShader = new Shader();
    gammaCorrectionShader->initFromFiles("../src/engine/fx/shaders/gamma.vert", "../src/engine/fx/shaders/gamma.frag");
    effectFrameBuffer = StatefulRenderer::createFrameBuffer(getApplication()->scaled_width(), getApplication()->scaled_height());
}

const FrameBuffer* GammaCorrectionEffect::apply(const FrameBuffer *sourceFrameBuffer, const Camera* camera) {
    float scaled_width = getApplication()->scaled_width();
    float scaled_height = getApplication()->scaled_height();
    MeshDrawData mdd;
    mdd.mesh = quadMesh;
    mdd.camera = camera;
    mdd.location = { scaled_width/2, scaled_height/2, -1};
    mdd.scale = { scaled_width, scaled_height, 1};
    mdd.texture = sourceFrameBuffer->texture();
    mdd.shader = std::shared_ptr<Shader>(gammaCorrectionShader);
    StatefulRenderer::activateFrameBuffer( effectFrameBuffer.get());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Renderer::drawMesh(mdd);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return effectFrameBuffer.get();

}

BloomEffect::BloomEffect() {
    float scaled_width = getApplication()->scaled_width();
    float scaled_height = getApplication()->scaled_height();
    bloomShader = std::make_shared<Shader>();
    bloomShader->initFromFiles("../src/engine/fx/shaders/bloom.vert", "../src/engine/fx/shaders/bloom.frag");
    quadShader = std::make_shared<Shader>();
    quadShader->initFromFiles("../src/engine/fx/shaders/quad.vert", "../src/engine/fx/shaders/quad.frag");
    gaussShader = std::make_shared<Shader>();
    gaussShader->initFromFiles("../src/engine/fx/shaders/gauss.vert", "../src/engine/fx/shaders/gauss.frag");
    using SF = StatefulRenderer;
    effectFrameBuffer = SF::createFrameBuffer(scaled_width, scaled_height, true);
    bloomFBO = SF::createFrameBuffer(scaled_width, scaled_height, true, true);
    auto blurFBO0 = SF::createFrameBuffer(scaled_width, scaled_height, true, false);
    auto blurFBO1 = SF::createFrameBuffer(scaled_width, scaled_height, true, false);
    blurFBOs.push_back(std::move(blurFBO0));
    blurFBOs.push_back(std::move(blurFBO1));

}

const FrameBuffer*  BloomEffect::apply(const FrameBuffer *sourceFrameBuffer, const Camera *camera) {
    // 1. render the bright parts of the source framebuffer into a separate framebuffer
    // 2. gaussian blur this (horizontal and vertical)
    // 3. Add the blurred texture to the source texture.

    // Step 1
    StatefulRenderer::activateFrameBuffer(bloomFBO.get());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    MeshDrawData mdd;
    mdd.camera = camera;
    mdd.mesh = quadMesh;
    mdd.shader = quadShader;
    float scaled_width = getApplication()->scaled_width();
    float scaled_height = getApplication()->scaled_height();
    mdd.location = { scaled_width/2, scaled_height/2, -5};
    mdd.scale = { scaled_width, scaled_height, 1};
    mdd.texture = sourceFrameBuffer->texture();
    Renderer::drawMesh(mdd);

    // Step 2: Do a gaussian blur on the brightness Framebuffer.
    // We alternate between horizontal and vertical blurs.
    mdd.camera = camera;
    mdd.mesh = quadMesh;
    mdd.shader = gaussShader;
    mdd.location = { scaled_width/2, scaled_height/2, -5};
    mdd.scale = { scaled_width, scaled_height, 1};
    mdd.texture = bloomFBO->texture2();

    glUseProgram(gaussShader->handle);
    bool horizontal = true, first_iteration = true;
    // Clear both blur framebuffers before first usage
    for (int i = 0; i < 2; i++) {
        StatefulRenderer::activateFrameBuffer(blurFBOs[i].get());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    int amount = 30;
    for (unsigned int i = 0; i < amount; i++)
    {
        StatefulRenderer::activateFrameBuffer(blurFBOs[horizontal].get());
        gaussShader->setIntValue(horizontal, "horizontal");
        mdd.texture = first_iteration ? bloomFBO->texture2() : blurFBOs[!horizontal]->texture();
        Renderer::drawMesh(mdd);;
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }

    // Step 3: addtivie blend the last blurred texture with the
    // incoming fullscreen texture. Return the resulting framebuffer as
    // our endresult.
    StatefulRenderer::activateFrameBuffer(effectFrameBuffer.get());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mdd.mesh = quadMesh;
    mdd.shader = bloomShader;
    mdd.location = { scaled_width/2, scaled_height/2, -5};
    mdd.scale = { scaled_width, scaled_height, 1};
    mdd.texture = sourceFrameBuffer->texture();
    // Index 0 is the last texturen written to, if the amount is even!
    blurFBOs[0]->texture()->bindAt(1);
    Renderer::drawMesh(mdd);

    return effectFrameBuffer.get();

}