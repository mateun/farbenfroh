//
// Created by mgrus on 25.03.2025.
//

#include "glm\glm.hpp"
#include <vector>
#include <cinttypes>


std::vector<uint32_t> flattenV3Indices(std::vector<glm::vec3> input) {
    std::vector<uint32_t> result;
    for (auto v : input) {
        result.push_back(v.x-1);
        result.push_back(v.y-1);
        result.push_back(v.z-1);

    }
    return result;
}

static std::vector<int32_t> flattenIV4(std::vector<glm::ivec4> input) {
    std::vector<int32_t> result;
    for (auto v : input) {
        result.push_back(v.x);
        result.push_back(v.y);
        result.push_back(v.z);
        result.push_back(v.w);

    }
    return result;
}

static std::vector<float> flattenV4(std::vector<glm::vec4> input) {
    std::vector<float> result;
    for (auto v : input) {
        result.push_back(v.x);
        result.push_back(v.y);
        result.push_back(v.z);
        result.push_back(v.w);

    }
    return result;
}

static std::vector<float> flattenV3(std::vector<glm::vec3> input) {
    std::vector<float> result;
    for (auto v : input) {
        result.push_back(v.x);
        result.push_back(v.y);
        result.push_back(v.z);

    }
    return result;
}

static std::vector<float> flattenV2(std::vector<glm::vec2> input) {
    std::vector<float> result;
    for (auto v : input) {
        result.push_back(v.x);
        result.push_back(v.y);
    }
    return result;
}
