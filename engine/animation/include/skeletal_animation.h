//
// Created by mgrus on 15.06.2025.
//

#ifndef SKELETAL_ANIMATION_H
#define SKELETAL_ANIMATION_H
#include <string>
#include <vector>
#include "glm/fwd.hpp"
#include "glm/detail/type_quat.hpp"

enum class ChannelType {
    translation,
    rotation,
    scale,
};

std::string getStringForChannelType(ChannelType type);


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
    std::string name = "unknown name";
    std::vector<KeyFrameChannel*> keyFrameChannels;
    std::vector<KeyFrameChannel*> keyFrameChannels_rotation;
    std::vector<KeyFrameChannel*> keyFrameChannels_translation;
    float duration = 0.0f;
};

std::vector<KeyFrameChannel*> getKeyFramesForJoint(SkeletalAnimation* animation, const std::string& jointName, ChannelType type);

#endif //SKELETAL_ANIMATION_H
