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
    Cinematic(Scene* scene, float duration);
    ~Cinematic();
    CineTrack* addTrack(SceneNode* node, const std::string& name);
    CineTrack* getTrack(const std::string& name);

    void play();
    float normalizedTime();
    void update();
    void stop();
    void pause();

    bool isActive();
    bool isOver();

    void render();

  private:
    std::map<std::string, CineTrack*> tracks;
    Scene * _scene = nullptr;
    bool _isPlaying = false;
    bool _hasFinished = false;
    float _localTime = 0.0f;
    float _duration = 0.0f;
};

/**
* A cinematic consists of tracks, which are assigned to an object (SceneNode).
* It has up to 3 channels: location, scale, rotation.
*/
class CineTrack {

  public:
    CineTrack(SceneNode* sceneNode);
    void addKeyFrame(ChannelType channelType, float time, glm::vec3 value, AngleUnit angleUnit = AngleUnit::DEGREES);

    std::vector<Channel*> getChannels();
    Channel* getChannel(ChannelType type);

    SceneNode* getNode() const;

    void applyInterpolatedTransform(float localTime, float normalizedTime);

    std::shared_ptr<SceneNode> _sceneNode = nullptr;

private:
  std::map<ChannelType, Channel*> _channels;
};

struct SampleValue {
  float time;
  glm::vec3 value;
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



#endif //CINEMATIC_H
