//
// Created by mgrus on 15.03.2025.
//

#ifndef COMPUTESHADER_H
#define COMPUTESHADER_H

#include <string>
#include <vector>
#include <GL/glew.h>
#include <stdexcept>
#include <glm\glm.hpp>

enum class DispatchOutput {
    Texture,
    Buffer
};

class Texture;

class ComputeShader {
public:
    ComputeShader(const std::string & shaderPath, std::vector<std::string> includeFiles = {});

    void initWithTexture(int width, int height);

    void use();

    void bindSSBO(GLuint ssbo) const;

    void dispatch(DispatchOutput dispatchOutput, glm::ivec3 groupSize) const;

    void setFloat(const std::string& uniformName, float time_passed);

private:
    // Different output options
    Texture * outputTexture = nullptr;
    GLuint handle = 0;
    GLuint ssbo = 0;
};



#endif //COMPUTESHADER_H
