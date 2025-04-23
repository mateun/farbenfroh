//
// Created by mgrus on 25.03.2025.
//

#include "Shader.h"
#include <engine/graphics/ErrorHandling.h>
#include <engine/io/io.h>
#include <glm/gtc/type_ptr.hpp>

void Shader::setVec4Value(const glm::vec4 vec, const std::string& name) {
    auto loc = glGetUniformLocation(this->handle, name.c_str());
    glUniform4f(loc, vec.x, vec.y, vec.z, vec.w);
    GL_ERROR_EXIT(442);
}

void Shader::setFloatValue(float val, const std::string &name) {
    auto loc = glGetUniformLocation(this->handle, name.c_str());
    glUniform1f(loc, val);
    GL_ERROR_EXIT(4430);
}

void Shader::setVec2Value(glm::vec<2, float> vec, const std::string &name) {
    auto loc = glGetUniformLocation(this->handle, name.c_str());
#ifdef _STRICT_SHADER_LOCATION_
    if (loc == -1) throw std::runtime_error("Could not get uniform location");
#endif
    glUniform2f(loc, vec.x, vec.y);
    GL_ERROR_EXIT(443);
}



void Shader::setVec3Value(glm::vec<3, float> vec, const std::string &name) {
    auto loc = glGetUniformLocation(this->handle, name.c_str());
#ifdef _STRICT_SHADER_LOCATION_
    if (loc == -1) throw std::runtime_error("Could not get uniform location");
#endif
    glUniform3f(loc, vec.x, vec.y, vec.z);
    GL_ERROR_EXIT(443);
}

void Shader::setMat4Value(glm::mat4 mat, const std::string &name) {
    auto e = glGetError();
    auto loc = glGetUniformLocation(this->handle, name.c_str());
#ifdef _STRICT_SHADER_LOCATION_
    if (loc == -1) throw std::runtime_error("Could not get uniform location");
#endif
    glUniformMatrix4fv(loc,1,  GL_FALSE, glm::value_ptr(mat));
    std::string errorInfo = name + " loc: " + std::to_string(loc);

    GL_ERROR_EXIT(444, errorInfo);
}

void Shader::setMat4Array(const std::vector<glm::mat4> mats, const std::string &name) {
    auto loc = glGetUniformLocation(this->handle, name.c_str());
#ifdef _STRICT_SHADER_LOCATION_
    if (loc == -1) throw std::runtime_error("Could not get uniform location");
#endif
    glUniformMatrix4fv(loc,mats.size(),  GL_FALSE, value_ptr(mats[0]));
    GL_ERROR_EXIT(445);
}

void Shader::setIntValue(int val, const std::string &name) {
    GL_ERROR_EXIT(4450);
    auto loc = glGetUniformLocation(this->handle, name.c_str());
    GL_ERROR_EXIT(4451);
#ifdef _STRICT_SHADER_LOCATION_
    if (loc == -1) throw std::runtime_error("Could not get uniform for int value: " + name);
#endif
    glUniform1i(loc, val);
    GL_ERROR_EXIT(445);

}


void Shader::initFromFiles(const std::string &vertFile, const std::string &fragFile) {
    auto vertSource = readFile(vertFile);
    auto fragSource = readFile(fragFile);

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vssource_char = vertSource.c_str();
    glShaderSource(vshader, 1, &vssource_char, NULL);
    glCompileShader(vshader);
    GLint compileStatus;
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &compileStatus);
    if (GL_FALSE == compileStatus) {
        std::cerr << "Error while compiling the vertex shader" << std::endl;

        GLint logSize = 0;
        glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &logSize);
        std::vector<GLchar> errorLog(logSize);
        glGetShaderInfoLog(vshader, logSize, &logSize, &errorLog[0]);
        //    result.errorMessage = errorLog.data();
        char buf[512];
        sprintf(buf, "vshader error: %s", errorLog.data());
        printf(buf);
        std::cerr << buf << std::endl;
        glDeleteShader(vshader);
        //  return result;


    }


    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fssource_char = fragSource.c_str();
    glShaderSource(fshader, 1, &fssource_char, NULL);
    glCompileShader(fshader);

    glGetShaderiv(fshader, GL_COMPILE_STATUS, &compileStatus);
    if (GL_FALSE == compileStatus) {
        GLint logSize = 0;
        glGetShaderiv(fshader, GL_INFO_LOG_LENGTH, &logSize);
        std::vector<GLchar> errorLog(logSize);
        glGetShaderInfoLog(fshader, logSize, &logSize, &errorLog[0]);
        //   result.errorMessage = errorLog.data();
        printf("fragment shader error: %s", errorLog.data());
        glDeleteShader(fshader);

    }

    GLuint p = glCreateProgram();
    glAttachShader(p, vshader);
    glAttachShader(p, fshader);
    glLinkProgram(p);

    glGetProgramiv(p, GL_LINK_STATUS, &compileStatus);

    if (GL_FALSE == compileStatus) {
        std::cerr << "Error during shader linking" << std::endl;
        GLint maxLength = 0;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(p, maxLength, &maxLength, &infoLog[0]);
        std::cerr << infoLog.data() << std::endl;
        printf("shader linking error: %s", infoLog.data());
        glDeleteProgram(p);
        glDeleteShader(vshader);
        glDeleteShader(fshader);
    }

    GL_ERROR_EXIT(9876);

    glDeleteShader(vshader);
    glDeleteShader(fshader);

    handle = p;

}

void Shader::bind() {
    glUseProgram(handle);
}

