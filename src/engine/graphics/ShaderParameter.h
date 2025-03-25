//
// Created by mgrus on 25.03.2025.
//

#ifndef SHADERPARAMETER_H
#define SHADERPARAMETER_H

#include <string>
#include <glm\glm.hpp>
#include <variant>

using ShaderParameterValue = std::variant<float, int, glm::vec2, glm::vec3, glm::vec4, glm::mat4>;


struct ShaderParameter {
    std::string name;
    ShaderParameterValue value;


};

#endif //SHADERPARAMETER_H
