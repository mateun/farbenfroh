//
// Created by mgrus on 15.03.2025.
//

#include "ComputeShader.h"
#include <GL/glew.h>

#include <graphics.h>
#include <engine/animation/Animation.h>
#include <engine/animation/Animation.h>
#include <engine/io/io.h>
#include <engine/profiling/PerformanceTimer.h>

ComputeShader::ComputeShader(const std::string &shaderPath, std::vector<std::string> includeFiles) {
    std::string source = "";

    // Put any includes in front of the main shader source file.
    for (auto includeFile : includeFiles) {
        auto includeSource = readFile(includeFile);
        source += includeSource;

    }

    // Add the actual shader core/main source:
    source += readFile(shaderPath);

    // Now compile and link the complete source:
    GLuint cshader = glCreateShader(GL_COMPUTE_SHADER);
    const GLchar* vssource_char = source.c_str();
    glShaderSource(cshader, 1, &vssource_char, NULL);
    glCompileShader(cshader);
    GLint compileStatus;
    glGetShaderiv(cshader, GL_COMPILE_STATUS, &compileStatus);
    if (GL_FALSE == compileStatus) {
        OutputDebugString("Error while compiling the compute shader\n");
        GLint logSize = 0;
        glGetShaderiv(cshader, GL_INFO_LOG_LENGTH, &logSize);
        std::vector<GLchar> errorLog(logSize);
        glGetShaderInfoLog(cshader, logSize, &logSize, &errorLog[0]);
        //    result.errorMessage = errorLog.data();
        char buf[4096];
        sprintf_s(buf, 4096, "compute shader error: %s", errorLog.data());
        printf(buf);
        OutputDebugStringA(buf);
        glDeleteShader(cshader);
        throw std::runtime_error(buf);

    }

    GLuint p = glCreateProgram();
    glAttachShader(p, cshader);
    glLinkProgram(p);

    glGetProgramiv(p, GL_LINK_STATUS, &compileStatus);

    if (GL_FALSE == compileStatus) {
        OutputDebugStringA("Error during compute shader linking\n");
        GLint maxLength = 0;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(p, maxLength, &maxLength, &infoLog[0]);
        OutputDebugStringA(infoLog.data());
        glDeleteProgram(p);
        glDeleteShader(cshader);
        char buf[1024];
        sprintf_s(buf, 1024, "compute shader linker error: %s", infoLog.data());
        printf(buf);
        throw std::runtime_error(buf);
    }

    glDeleteShader(cshader);
    handle = p;
    GL_ERROR_EXIT(10100)

}


void ComputeShader::initWithTexture(int width, int height) {
    // Create our output texture, where the compute shader writes to
    const int texWidth = width;
    const int texHeight = height;
    GLuint outputTex;
    glGenTextures(1, &outputTex);
    glBindTexture(GL_TEXTURE_2D, outputTex);
    glTextureStorage2D(outputTex, 1, GL_RGBA32F, texWidth, texHeight);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Bind the texture to image unit 0 (so it matches the compute shader's binding = 0).
    // We use GL_WRITE_ONLY because the compute shader is writing to the image.
    glBindImageTexture(0, outputTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    outputTexture = new Texture();
    outputTexture->handle = outputTex;


}

void ComputeShader::use() {
    glUseProgram(handle);
}

void ComputeShader::bindSSBO(GLuint ssbo) const {

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
}

void ComputeShader::dispatch(DispatchOutput dispatchOutput, glm::ivec3 groupSize) const {

    glUseProgram(handle);
    // if (dispatchOutput == DispatchOutput::Buffer) {
    //     bindSSBO();
    // } else {
    //     glBindImageTexture(0, outputTexture->handle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    // }

    // glDispatchCompute uses the number of work groups. Our local group size is 1x1,
    // so we need the same dimension as our texture to cover every pixel once.
    auto timer = PerformanceTimer(true);
    glDispatchCompute(groupSize.x, groupSize.y, groupSize.z);
    // Use memory barrier to wait until completion.
    if (dispatchOutput == DispatchOutput::Buffer) {
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    } else {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
    timer.stop();
#ifdef PROFILING
    printf("compute shader duration elapsed: %3.2f micros\n", timer.durationInSeconds() * 1000 * 1000);
#endif
}

void ComputeShader::setFloat(const std::string &uniformName, float value) {
    glUseProgram(handle);
    auto loc = glGetUniformLocation(handle, uniformName.c_str());
    GL_ERROR_EXIT(10200)
    glUniform1f(loc, value);
    GL_ERROR_EXIT(10201)
}

// End compute shader testing
