//
// Created by mgrus on 23.04.2025.
//

#ifndef OPENGL46_H
#define OPENGL46_H

#include <symbol_exports.h>

#include "../renderer/include/renderer.h"
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
 * Implemetns the FramgementShaderBuilder interface for OpenGL 4.6.
 */
class GL46FramgentShaderBuilder : public renderer::FragmentShaderBuilder {
public:
    FragmentShaderBuilder &color() override;
    FragmentShaderBuilder &diffuseTexture(uint8_t textureUnit, bool flipUVs) override;
    FragmentShaderBuilder &textRender() override;
    std::string build() const override;

private:
    bool useTextRender = false;
    bool useColor = false;
    bool useDiffuseTexture = false;
    uint8_t diffuseTextureUnit = 0;
    bool flipUVs = false;
};

/**
 * Implemetns the VertexBufferBuilder interface for OpenGL 4.6.
 */
class GL46VertexBufferBuilder : public renderer::VertexBufferBuilder {
public:
    VertexBufferBuilder &attributeVec3(renderer::VertexAttributeSemantic semantic, const std::vector<glm::vec3> &data) override;
    VertexBufferBuilder &attributeVec2(renderer::VertexAttributeSemantic semantic, const std::vector<glm::vec2> &data) override;
    renderer::VertexBufferHandle build() const override;

private:

    template<typename T>
    VertexBufferBuilder &attributeT(renderer::VertexAttributeSemantic semantic, const std::vector<T> &data) ;

    std::vector<renderer::VertexAttribute> attributes_;
    std::vector<float> raw_data_;
    size_t current_stride_ = 0;

    size_t element_size_ = 0;
    size_t element_count_ = 0;
};

/**
 * Implements the VertexSahderBuilder interface for OpenGL 4.6.
 */
class GL46VertexShaderBuilder : public renderer::VertexShaderBuilder {
public:
    GL46VertexShaderBuilder& position(uint8_t slot) override {
        hasPosition = true;
        positionSlot = slot;
        return *this;
    }

    GL46VertexShaderBuilder& normal(uint8_t slot) override {
        hasNormal = true;
        normalSlot = slot;
        return *this;
    }

    GL46VertexShaderBuilder& uv(uint8_t slot) override {
        hasUV = true;
        uvSlot = slot;
        return *this;
    }

    VertexShaderBuilder &mvp() override {
        hasMVPUniforms = true;
        return *this;
    }


    VertexShaderBuilder &worldMatrix() override {
        hasWorldMatrixUniform = true;
        return *this;
    }

    VertexShaderBuilder &projectionMatrix() override {
        hasWorldMatrixUniform = true;
        return *this;
    }

    VertexShaderBuilder &viewMatrix() override {
        hasWorldMatrixUniform = true;
        return *this;
    }








    [[nodiscard]] std::string build() const override {
        std::string src = "#version 460 core\n";

        // Declare vertex attributes:
        if (hasPosition)
            src += "layout(location = " + std::to_string(positionSlot) + ") in vec3 aPosition;\n";
        if (hasNormal)
            src += "layout(location = " + std::to_string(normalSlot) + ") in vec3 aNormal;\n";
        if (hasUV)
            src += "layout(location = " + std::to_string(uvSlot) +") in vec2 aUV;\n";

        // Declare uniforms:
        if (hasMVPUniforms) {
            src += "uniform mat4 mvpMatrix;\n";
        }

        if (hasWorldMatrixUniform) {
            src += "uniform mat4 world_mat;\n";
        }

        if (hasProjectionMatrixUniform) {
            src += "uniform mat4 proj_mat;\n";
        }

        if (hasViewMatrixUniform) {
            src += "uniform mat4 view_mat;\n";
        }

        std::string mvpPart = "";
        if (hasMVPUniforms) {
            mvpPart += "mvpMatrix *";
        }
        if (hasWorldMatrixUniform) {
            mvpPart += "world_mat *";
        }

        if (hasProjectionMatrixUniform) {
            mvpPart += "proj_mat *";
        }

        if (hasViewMatrixUniform) {
            mvpPart += "view_mat *";
        }

        // Declare outputs:
        if (hasUV) {
            src += "out vec2 fs_uvs;\n";
        }

        src += "void main() {\n";

        // Calculate clip-space position:
        if (hasPosition) {
            src += "\tgl_Position = " + mvpPart + " vec4(aPosition, 1.0);\n";
        }
        else {
            src += "    gl_Position = vec4(0.0);\n";
        }

        // Assign other outputs:
        if (hasUV) {
            src += "    fs_uvs = aUV;\n";
            // TODO handle uv flipping
            //src += "    fs_uvs.y = 1 - fs_uvs.y;\n";
        }

        src += "}\n";

        return src;
    }

private:
    bool hasPosition = false;
    bool hasNormal = false;
    bool hasUV = false;
    uint8_t positionSlot = 0;
    uint8_t normalSlot = 0;
    uint8_t uvSlot = 0;
    bool hasMVPUniforms = false;
    bool hasWorldMatrixUniform;
    bool hasProjectionMatrixUniform;
    bool hasViewMatrixUniform;
};






extern "C" ENGINE_API void initOpenGL46(HWND, bool useSRGB = false, int msaaSampleCount = 0);



#endif //OPENGL46_H
