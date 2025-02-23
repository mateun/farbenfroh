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

void CineTrack::addKeyFrame(ChannelType channelType, float time, glm::vec3 value) {
    auto channel = _channels[channelType];
    if (!channel) {
        channel = new Channel(channelType);
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
        interpolatedRotation = rotationChannel->getInterpolatedSampleValue( localTime, normalizedTime);
        _sceneNode->setRotation(glm::eulerAngles(interpolatedRotation));
    }



}

Channel::Channel(ChannelType type) : _type(type) {
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

    if (this->_type == ChannelType::Location) {
        auto val =  glm::mix(fromSample.value().value, toSample.value().value, timeNormalized);
        return val;
    }

    if (this->_type == ChannelType::Scale) {
        auto ival = glm::mix(fromSample.value().value, toSample.value().value, timeNormalized);
        return ival;
    }

    if (this->_type == ChannelType::Rotation) {
        glm::quat fromQuat = glm::qua(fromSample.value().value);
        glm::quat toQuat = glm::qua(toSample.value().value);
        auto interpRotation = slerp(fromQuat, toQuat, timeNormalized);
        return eulerAngles(interpRotation);
    }

}
