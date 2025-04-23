//
// Created by mgrus on 17.02.2025.
//

#ifndef ANIMATION_H
#define ANIMATION_H

#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <glm\glm.hpp>
#include "glm/detail/type_quat.hpp"

struct Joint;
class JointMask;

enum class SampleType {
    translation,
    rotation,
    scale
};

struct AnimationSample {
    SampleType type;
    std::string jointName;
    float time;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 scale;
};

class SampleStore {
public:
    std::vector<AnimationSample*> findTranslationSamples(const std::string &jointName);
    std::vector<AnimationSample*> findRotationSamples(const std::string &jointName);
    std::vector<AnimationSample*> findScaleSamples(const std::string &jointName);

    void store(AnimationSample * sample, const std::string& jointName);

    std::vector<AnimationSample *> allTranslationSamples();
    std::vector<AnimationSample *> allRotationSamples();
    std::vector<AnimationSample *> allScaleSamples();

private:
    std::map<std::string,std::vector<AnimationSample*>> _translationSamples;
    std::map<std::string,std::vector<AnimationSample*>> _rotationSamples;
    std::map<std::string,std::vector<AnimationSample*>> _scaleSamples;
};

class Animation {

public:
    Animation();

    void storeSample(AnimationSample* sample, const std::string& jointName);
    std::vector<AnimationSample*> findSamples(SampleType sampleType);
    std::vector<AnimationSample*> findSamples(const std::string& jointName, SampleType sampleType);
    void applyJointMask(JointMask* m);

    /**
    * Checks if the joint is masked by any applied masks.
    */
    bool isJointMasked(Joint* j);

    bool isMasked();

    std::string name;
    float duration;
    int frames;
    SampleStore* sampleStore = nullptr;
    double ticksPerSecond;
    float currentDebugTimestamp = 0;
    std::string currentLayerMask;
    std::unordered_set<JointMask*> jointMasks;
};



#endif //ANIMATION_H
