//
// Created by mgrus on 23.04.2025.
//

#ifndef OPENGL46_H
#define OPENGL46_H

#include <symbol_exports.h>
//#include <GL/glew.h>
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

class GL46FramgentShaderBuilder : public renderer::FragmentShaderBuilder {
    FragmentShaderBuilder &color() override;
    std::string build() const override;
private:
    bool useColor = false;
};

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

class GL46VertexShaderBuilder : public renderer::VertexShaderBuilder {
public:
    GL46VertexShaderBuilder& position() override {
        hasPosition = true;
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




    GL46VertexShaderBuilder& normal() override {
        hasNormal = true;
        return *this;
    }

    GL46VertexShaderBuilder& uv() override {
        hasUV = true;
        return *this;
    }



    [[nodiscard]] std::string build() const override {
        std::string src = "#version 460 core\n";

        if (hasPosition)
            src += "layout(location = 0) in vec3 aPosition;\n";
        if (hasNormal)
            src += "layout(location = 1) in vec3 aNormal;\n";
        if (hasUV)
            src += "layout(location = 2) in vec2 aUV;\n";

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

        src += "void main() {\n";

        if (hasPosition) {
            src += "\tgl_Position = " + mvpPart + " vec4(aPosition, 1.0);\n";
        }

        else
            src += "    gl_Position = vec4(0.0);\n";

        src += "}\n";

        return src;
    }

private:
    bool hasPosition = false;
    bool hasNormal = false;
    bool hasUV = false;
    bool hasMVPUniforms = false;
    bool hasWorldMatrixUniform;
    bool hasProjectionMatrixUniform;
    bool hasViewMatrixUniform;
};


extern "C" ENGINE_API void initOpenGL46(HWND);



#endif //OPENGL46_H
