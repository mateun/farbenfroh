//
// Created by mgrus on 16.02.2025.
//

#include "AnimationController.h"
#include "AnimationState.h"
#include "AnimationTransition.h"

AnimationController::AnimationController() {
//    _player = new AnimationPlayer(nullptr, nullptr);
}

AnimationController::~AnimationController() {
}

void AnimationController::addAnimationState(AnimationState *animationState) {
    _animationStates.push_back(animationState);
}

void AnimationController::update() {
    if (_currentState == nullptr) {
        _currentState = _animationStates[0];
  //      _player->switchAnimation(_currentState->getAnimation());
    }
    auto outgoingTransitions = _currentState->getOutgoingTransitions();
    for (auto transition : outgoingTransitions) {
        if (transition->evaluate()) {
            _currentState = transition->getEndState();
        }
    }

//    _player->update();
}

std::vector<glm::mat4> AnimationController::getBoneMatrices() {
    //return _player->getCurrentBoneMatrices();
    return {};
}

std::optional<AnimationProperty> AnimationController::getProperty(const std::string &key) {
    if (!properties.contains(key)) {
        return std::nullopt;
    }

    return properties[key];

}

void AnimationController::setProperty(const std::string key, AnimationProperty animationProperty) {
    properties[key] = animationProperty;
}

AnimationState * AnimationController::getCurrentState() {
    return _currentState;
}
