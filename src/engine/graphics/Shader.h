//
// Created by mgrus on 25.03.2025.
//

#ifndef SHADER_H
#define SHADER_H

#include <GL\glew.h>
#include <string>
#include <glm\glm.hpp>
#include <vector>

class Shader {
public:
    GLuint handle;

    void setIntValue(int val, const std::string &name);
    void setFloatValue(float value, const std::string& name);
    void setVec2Value(glm::vec<2, float> vec, const std::string &name);
    void setVec3Value(glm::vec<3, float> vec, const std::string& name);
    void setVec4Value(glm::vec4 vec, const std::string& name);
    void setMat4Value(glm::mat4 mat, const std::string& str);
    void setMat4Array(std::vector<glm::mat4> mats, const std::string& name);

    void initFromFiles(const std::string& vertexShader, const std::string& fragmentShader);

    void bind();
};

#endif //SHADER_H
