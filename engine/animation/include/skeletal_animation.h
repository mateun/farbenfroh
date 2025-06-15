//
// Created by mgrus on 15.06.2025.
//

#ifndef SKELETAL_ANIMATION_H
#define SKELETAL_ANIMATION_H
#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

enum class ChannelType {
    translation,
    rotation,
    scale,
};

struct KeyFrameChannel {
    ChannelType type;
    float time;
    std::string joint_name;
    glm::vec3 value_v3;
    glm::vec4 value_v4;
    glm::quat value_quat;
    float value_f;

};

struct SkeletalAnimation {
    std::string name;
    std::vector<KeyFrameChannel> keyFrameChannels;
};

#endif //SKELETAL_ANIMATION_H
