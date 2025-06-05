//
// Created by mgrus on 23.04.2025.
//

#ifndef OPENGL46_H
#define OPENGL46_H


#include <renderer.h>
#include <gl/gl.h>
#include <cinttypes>




struct GLTexture {
    GLuint id;
    uint32_t width;
    uint32_t height;
    GLenum format;
};

struct GLShader {
    GLuint id;
};

struct GLProgram {
    GLuint id;
};

struct GLVao {
    GLuint id;
};
struct GLVbo {
    GLuint id;
    GLenum format;
};

/**
 * Implemetns the RenderTargetBuilder interface for OpenGL 4.6.
 */
class GL46RenderTargetBuilder: public renderer::RenderTargetBuilder {
public:
    RenderTargetBuilder &size(int w, int h) override;
    RenderTargetBuilder &color() override;
    RenderTargetBuilder &depth() override;
    RenderTargetBuilder &stencil() override;
    renderer::RenderTarget build() override;

private:
    int width = 1024;
    int height = 1024;
    bool useDepth = false;
    bool useStencil = false;
    bool usesColor = false;
};


/**
 * Implemetns the VertexBufferBuilder interface for OpenGL 4.6.
 */
class GL46VertexBufferBuilder : public renderer::VertexBufferBuilder {
public:
    VertexBufferBuilder &attributeVec3(renderer::VertexAttributeSemantic semantic, const std::vector<glm::vec3> &data) override;
    VertexBufferBuilder &attributeVec2(renderer::VertexAttributeSemantic semantic, const std::vector<glm::vec2> &data) override;
    renderer::VertexBufferHandle build() const override;
    void update(renderer::VertexBufferHandle existingVBO) const override;

private:

    template<typename T>
    VertexBufferBuilder &attributeT(renderer::VertexAttributeSemantic semantic, const std::vector<T> &data) ;

    renderer::VertexBufferCreateInfo commonUpdateBuild() const;

    std::vector<renderer::VertexAttribute> attributes_;
    std::vector<float> raw_data_;
    size_t current_stride_ = 0;

    size_t element_size_ = 0;
    size_t element_count_ = 0;
};


void initOpenGL46(HWND, bool useSRGB = false, int msaaSampleCount = 0);



#endif //OPENGL46_H
