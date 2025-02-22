//
// Created by mgrus on 22.02.2025.
//

#include "Cinematic.h"

Cinematic::Cinematic(Scene *scene) : _scene(scene) {

}

Cinematic::~Cinematic() {
}

CineTrack * Cinematic::addTrack(SceneNode *node, const std::string &name) {
    auto track = new CineTrack(node);
    tracks[name] = track;
}

CineTrack * Cinematic::getTrack(const std::string &name) {
    return tracks[name];
}

void Cinematic::play() {
    _isPlaying = true;
}

/**
* For the current local time, we interpolate between the last and next keyframe for every track/channel.
* The resulting locations/rotations/scalings are applied to the respective track objects.
*/
void Cinematic::update() {
    _localTime += ftSeconds;
    for (auto nameTrackPair : tracks) {
        auto track = nameTrackPair.second;
        for (auto channel : track.getChannels()) {

        }
    }


}

Channel::Channel(ChannelType type) : _type(type) {
}
