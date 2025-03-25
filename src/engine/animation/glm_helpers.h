//
// Created by mgrus on 25.03.2025.
//

#ifndef GLM_HELPERS_H
#define GLM_HELPERS_H

#include <cinttypes>
#include <vector>
#include <glm/glm.hpp>

std::vector<uint32_t> flattenV3Indices(std::vector<glm::vec3> input);

std::vector<int32_t> flattenIV4(std::vector<glm::ivec4> input);

std::vector<float> flattenV4(std::vector<glm::vec4> input);

std::vector<float> flattenV3(std::vector<glm::vec3> input);

std::vector<float> flattenV2(std::vector<glm::vec2> input);


#endif //GLM_HELPERS_H
