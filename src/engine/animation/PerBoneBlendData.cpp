//
// Created by mgrus on 08.03.2025.
//

#include "PerBoneBlendData.h"

void PerBoneBlendData::addNamedBoneWeight(const std::string &boneName, float weight) {
    boneWeights[boneName] = weight;
}

std::optional<float> PerBoneBlendData::getWeightForBone(const std::string& boneName) {
    auto it = boneWeights.find(boneName);
    if (it != boneWeights.end()) {
        return it->second;
    }
    return std::nullopt;
}
