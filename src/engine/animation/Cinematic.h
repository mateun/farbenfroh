//
// Created by mgrus on 22.02.2025.
//

#ifndef CINEMATIC_H
#define CINEMATIC_H
#include <string>
#include <map>

class Scene;
class SceneNode;
class CineTrack;
class Channel;
enum class ChannelType {
  Location,
  Rotation,
  Scale
};


/**
* Can be played in a game in realtime.
* It has different object tracks, e.g. for camera or other SceneNodes.
* Every track has different chennels for location, scale, rotation.
* The cinematic is the sum of all tracks and channels over a given time.
* E.g. a city cinematic consists of a scene with a bunch of houses, cars etc.
* The main camera is assigned to a track and has a few keyframes to capture different views of the city.
* (by the means of rotation key frames).
* In addition, a car and a pedestrian occopy a track each, having location and rotation key frames in the respective channels.
* Example usage:
* cine1 = new Cinematic();
* cine1.setScene(scene1);
* camTrack = cine1.addTrackForObject(mainCam);
* carTrack = cine1.addTrackForObject(car3);
* pedestrianTrack = cine1.addTrackForObject(pedestrian41);
* cine1.addKeyFrameToTrack(camTrack, timeInSeconds, channelType, myrotQuat);
* ...
* adding all kinds of keyframes.
* // Play activates the cinematic.
* cine1.play();
* // Calling update each frame to move the cinematic forward:
* cine1.update();
*
*/
class Cinematic {

  public:
    Cinematic(Scene* scene);
    ~Cinematic();
    CineTrack* addTrack(SceneNode* node, const std::string& name);
    CineTrack* getTrack(const std::string& name);

    void play();
    void update();
    void stop();
    void pause();

  private:
    std::map<std::string, CineTrack*> tracks;
    Scene * _scene = nullptr;
    bool _isPlaying = false;
    float _localTime = 0.0f;
};

/**
* A cinematic consists of tracks, which are assigned to an object (SceneNode).
* It has up to 3 channels: location, scale, rotation.
*/
class CineTrack {

  public:
    CineTrack(SceneNode* sceneNode);
    void addKeyFrame(SceneNode* node);

    std::vector<Channel*> getChannels();
};

class Channel {
public:
  Channel(ChannelType type);

  /**
  * This method shall interpolate between two keyframes for its own channel
  * and apply the resulting (location/roation/scale) to the given scene node.
  */
  void applyTransform(SceneNode* node, float time);

private:
  ChannelType _type;
};



#endif //CINEMATIC_H
