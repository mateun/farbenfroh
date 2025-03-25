//
// Created by mgrus on 25.03.2025.
//

#ifndef CHANNEL_H
#define CHANNEL_H

#include <map>
#include <glm/glm.hpp>
#include <engine/math/data_structures.h>

class SampleValue;
class CineTrack;


enum class ChannelType {
    Location,
    Rotation,
    Scale
  };




class Channel {
    friend class CineTrack;

public:
    Channel(ChannelType type, AngleUnit unit = AngleUnit::DEGREES);

private:
    ChannelType _type;
    // This value is polymorphic in a way:
    // depending on the type it can mean different things:
    // location, translation, scale
    // The map goes from a discrete time value to a specific sample
    std::map<float, SampleValue> _samples;

    glm::vec3 getInterpolatedSampleValue(float timeAbsolute, float timeNormalized);

    void addKeyFrame(float time, glm::vec3 vec);

    AngleUnit _angleUnit;
};


#endif //CHANNEL_H
