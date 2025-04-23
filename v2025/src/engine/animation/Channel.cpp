//
// Created by mgrus on 25.03.2025.
//

#include <optional>
#include <ranges>
#include <vector>
#include "Channel.h"
#include "Animation.h"
#include "Cinematic.h"
#include <glm\gtc/quaternion.hpp>



Channel::Channel(ChannelType type, AngleUnit angleUnit) : _type(type), _angleUnit(angleUnit) {
}

void Channel::addKeyFrame(float time, glm::vec3 value) {
    _samples[time] = {time, value};
}

/**
* This method gets the surrounding samples for the given point in time
* and then interpolates the values.
* The interpolated transformation is then returned from the function.
*/
glm::vec3 Channel::getInterpolatedSampleValue(float timeAbsolute, float timeNormalized) {
    std::optional<SampleValue> fromSample = std::nullopt;
    std::optional<SampleValue> toSample = std::nullopt;

    auto allSamples = _samples | std::views::values;
    auto allSamplesVec = std::vector(allSamples.begin(), allSamples.end());
    int index = 0;
    for (auto sample :allSamples) {
        printf("%f %f\n", timeAbsolute, sample.time);
        if (timeAbsolute < allSamplesVec[index+1].time) {
            fromSample = sample;
            break;
        }

        index++;

    }

    if (allSamples.size() > index+1) {
        toSample = allSamplesVec[index+1];
    } else {
        // TODO if we do not have a good end index
        throw std::runtime_error("oh no, no more index!");
    }

    float diffTime = toSample->time - fromSample->time;
    float timePart = timeAbsolute - fromSample->time;
    float blendFactor = timePart / diffTime;

    if (this->_type == ChannelType::Location) {
        auto val =  glm::mix(fromSample.value().value, toSample.value().value, blendFactor);
        return val;
    }

    if (this->_type == ChannelType::Scale) {
        auto ival = glm::mix(fromSample.value().value, toSample.value().value, blendFactor);
        return ival;
    }

    if (this->_type == ChannelType::Rotation) {
        auto fromVal = _angleUnit == AngleUnit::RAD ? fromSample.value().value : radians(fromSample.value().value);
        auto toVal = _angleUnit == AngleUnit::RAD ? toSample.value().value : radians(toSample.value().value);
        glm::quat fromQuat = glm::qua(fromVal);
        glm::quat toQuat = glm::qua(toVal);
        auto interpRotation = slerp(fromQuat, toQuat, blendFactor);

        return _angleUnit == AngleUnit::RAD ? eulerAngles(interpRotation) : degrees(eulerAngles(interpRotation));
    }
    return {0.0f, 0.0f, 0.0f};

}