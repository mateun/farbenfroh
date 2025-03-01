//
// Created by mgrus on 22.02.2025.
//

#include "Cinematic.h"

#include <ranges>

Cinematic::Cinematic(Scene *scene, float duration) : _scene(scene) , _duration(duration) {
}

Cinematic::~Cinematic() {
}

CineTrack * Cinematic::addTrack(SceneNode *node, const std::string &name) {
    auto track = new CineTrack(node);
    tracks[name] = track;
    return track;
}

CineTrack * Cinematic::getTrack(const std::string &name) {
    return tracks[name];
}

void Cinematic::play() {
    _isPlaying = true;
    _hasFinished = false;
}

/**
* Creates the current amount of localtime relative to the duration,
* resulting in a value of 0 - 1. 0 = beginning, 1 the end of the whole animation.
* Useful for e.g. animation interpolation.
*/
float Cinematic::normalizedTime() {
    float nt =  _localTime / _duration;
    return nt;
}

/**
* For the current local time, we interpolate between the last and next keyframe for every track/channel.
* The resulting locations/rotations/scalings are applied to the respective track objects.
*/
void Cinematic::update() {
    if (!_isPlaying) {
        return;
    }

    _localTime += ftSeconds;

    if (_localTime >= _duration) {
        _isPlaying = false;
        _localTime -= _duration;
        _hasFinished = true;
        return;
    }

    for (auto track : tracks | std::ranges::views::values) {
        track->applyInterpolatedTransform(_localTime, normalizedTime());



    }


}

void Cinematic::stop() {
    _isPlaying = false;
    _localTime = 0;
}

void Cinematic::pause() {
    _isPlaying = false;
}

bool Cinematic::isActive() {
    return _isPlaying;
}

/**
* To be "over", the cinematic must have been played and
* has been playing until its end duration.
*/
bool Cinematic::isOver() {
    return _hasFinished;
}

/**
* Renders the cinematic based on the current local time.
* First, we need to find our current camera.
* In case we do have a camera track (or more...), this overrides the
* main camera in the scene.
* We will then render from this animated track camera.
* Otherwise we just use the main scene camera.
*
* For any other scene object, we check if it has a track.
* If yes, we calculate the transform for the current time and render the object
* with this transform.
* If there is no track for the given object, we just render the static version
* of the SceneNode as is.
*/
[[Deprecated("Question: not needed as the scene incl. camera(s) gets manipulated during update anyway?!")]]
void Cinematic::render() {

    auto camNode = _scene->findActiveCameraNode();
    for (auto track : tracks | std::ranges::views::values) {
        if (track->getNode() == camNode) {
            // We need to find the interpolated transformation of the camera!
            track->applyInterpolatedTransform(_localTime, normalizedTime());
        }
    }

    _scene->render();

}

CineTrack::CineTrack(SceneNode *sceneNode) : _sceneNode(sceneNode){
}

void CineTrack::addKeyFrame(ChannelType channelType, float time, glm::vec3 value, AngleUnit angleUnit) {
    auto channel = _channels[channelType];
    if (!channel) {
        channel = new Channel(channelType, angleUnit);
        _channels[channelType] = channel;
    }
    channel->addKeyFrame(time, value);
}

std::vector<Channel *> CineTrack::getChannels() {
    std::vector<Channel *> channels;
    for (auto &val: _channels | std::views::values) {
        channels.push_back(val);
    }
    return channels;
}

Channel * CineTrack::getChannel(ChannelType type) {
    return _channels[type];
}

SceneNode * CineTrack::getNode() {
    return _sceneNode;
}

void CineTrack::applyInterpolatedTransform(float localTime, float normalizedTime) {
    glm::vec3 interpolatedLocation = glm::vec3(0);
    glm::quat interpolatedRotation;

    auto locationChannel = getChannel(ChannelType::Location);
    auto rotationChannel = getChannel(ChannelType::Rotation);

    if (locationChannel) {
        interpolatedLocation = locationChannel->getInterpolatedSampleValue( localTime, normalizedTime);
        _sceneNode->setLocation(interpolatedLocation);
    }

    if (rotationChannel) {
        auto interpRot = rotationChannel->getInterpolatedSampleValue( localTime, normalizedTime);
        _sceneNode->setRotation(interpRot, rotationChannel->_angleUnit);
    }



}

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
