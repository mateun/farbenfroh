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

class GL46VertexShaderBuilder : public renderer::VertexShaderBuilder {
public:
    GL46VertexShaderBuilder& position() override {
        hasPosition = true;
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

        src += "void main() {\n";

        if (hasPosition)
            src += "    gl_Position = vec4(aPosition, 1.0);\n";
        else
            src += "    gl_Position = vec4(0.0);\n";

        src += "}\n";

        return src;
    }

private:
    bool hasPosition = false;
    bool hasNormal = false;
    bool hasUV = false;
};


extern "C" ENGINE_API void initOpenGL46(HWND);



#endif //OPENGL46_H
