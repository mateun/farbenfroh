//
// Created by mgrus on 26.03.2025.
//

#ifndef POSTPROCESSEFFECT_H
#define POSTPROCESSEFFECT_H

#include <memory>
#include <engine/graphics/FrameBuffer.h>
#include <engine/graphics/Mesh.h>
#include <engine/graphics/Shader.h>
#include <engine/graphics/Texture.h>
#include <engine/graphics/Camera.h>

class PostProcessEffect {
public:
    PostProcessEffect();
    virtual const FrameBuffer* apply(const FrameBuffer* sourceFrameBuffer, const Camera* camera) = 0;

protected:
    std::shared_ptr<Mesh> quadMesh;
    std::shared_ptr<FrameBuffer> effectFrameBuffer;
};

class GammaCorrectionEffect : public PostProcessEffect {
public:
    GammaCorrectionEffect();
    const FrameBuffer* apply(const FrameBuffer* sourceFrameBuffer, const Camera* camera) override;

private:
    Shader* gammaCorrectionShader = nullptr;

};

class BloomEffect : public PostProcessEffect {
public:
    BloomEffect();
    const FrameBuffer* apply(const FrameBuffer *sourceFrameBuffer, const Camera* camera) override;

private:
    std::shared_ptr<Shader> bloomShader;
    std::shared_ptr<Shader> quadShader;
    std::shared_ptr<Shader> gaussShader;
    std::shared_ptr<FrameBuffer> bloomFBO = nullptr;
    std::vector<std::unique_ptr<FrameBuffer>> blurFBOs;
};


#endif //POSTPROCESSEFFECT_H
