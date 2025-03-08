//
// Created by mgrus on 19.02.2025.
//
#include "Animation.h"

#include <ranges>

#include "JointMask.h"

std::vector<AnimationSample*> SampleStore::findTranslationSamples(const std::string &jointName) {
  return _translationSamples[jointName];
}

std::vector<AnimationSample *> SampleStore::findRotationSamples(const std::string &jointName) {
  return _rotationSamples[jointName];
}

std::vector<AnimationSample *> SampleStore::findScaleSamples(const std::string &jointName) {
  return _scaleSamples[jointName];
}

void SampleStore::store(AnimationSample *sample, const std::string &jointName) {
  if (sample->type == SampleType::translation) {
    _translationSamples[jointName].push_back(sample);
  }
  if (sample->type == SampleType::rotation) {
    _rotationSamples[jointName].push_back(sample);
  }
  if (sample->type == SampleType::scale) {
    _scaleSamples[jointName].push_back(sample);
  }
}

std::vector<AnimationSample *> retrieveAllFromMap(std::map<std::string, std::vector<AnimationSample *>> &map) {
  std::vector<AnimationSample*> values;
  for (const auto& pair : map) {
    for (const auto& sample : pair.second) {
      values.push_back(sample);
    }
  }
  return values;
}

std::vector<AnimationSample *> SampleStore::allTranslationSamples() {
  return retrieveAllFromMap(_translationSamples);
}

std::vector<AnimationSample *> SampleStore::allRotationSamples() {
  return retrieveAllFromMap(_rotationSamples);
}

std::vector<AnimationSample *> SampleStore::allScaleSamples() {
  return retrieveAllFromMap(_scaleSamples);
}

Animation::Animation() {
  this->sampleStore = new SampleStore();
}

void Animation::storeSample(AnimationSample *sample, const std::string& jointName) {
  sampleStore->store(sample, jointName);
}

std::vector<AnimationSample *> Animation::findSamples(SampleType sampleType) {
  if (sampleType == SampleType::translation) {
    return sampleStore->allTranslationSamples();
  }
  if (sampleType == SampleType::rotation) {
    return sampleStore->allRotationSamples();
  }
  if (sampleType == SampleType::scale) {
    return sampleStore->allScaleSamples();
  }
  return {};
}

std::vector<AnimationSample*> Animation::findSamples(const std::string &jointName, SampleType sampleType) {
  if (sampleType == SampleType::translation) {
    return sampleStore->findTranslationSamples(jointName);
  }
  if (sampleType == SampleType::rotation) {
    return sampleStore->findRotationSamples(jointName);
  }
  if (sampleType == SampleType::scale) {
    return sampleStore->findScaleSamples(jointName);
  }

  return {};
}

void Animation::applyJointMask(JointMask *m) {
  this->jointMasks.emplace(m);
}

bool Animation::isJointMasked(Joint *j) {
  if (!isMasked()) { return false; }

  bool masked = false;
  for (auto m : jointMasks) {
    masked = m->isPartOfMask(j);
    if (masked) {
      // Bail out quickly for the first mask hit.
      return true;
    }
  }

  return masked;
}

bool Animation::isMasked() {
  return !jointMasks.empty();
}

