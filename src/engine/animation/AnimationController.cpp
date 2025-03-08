//
// Created by mgrus on 16.02.2025.
//

#include "AnimationController.h"
#include "AnimationState.h"
#include "AnimationTransition.h"

AnimationController::AnimationController(Mesh* mesh) : _mesh(mesh) {
    _player = new AnimationPlayer((Animation*)nullptr, mesh);
}

AnimationController::~AnimationController() {
}

void AnimationController::addAnimationState(AnimationState *animationState) {
    _animationStates.push_back(animationState);
}

void AnimationController::update() {
    if (_currentState == nullptr) {
        _currentState = _animationStates[0];
        _player->switchAnimation(_currentState->getAnimation());
        _player->play(true);    // TODO handle looping

    }

    // Check if we need to go through a transition.
    // Based on the current state, we check the first transition which evaluates to true,
    // and set it as the current transition.
    if (!_currentTransition) {
        auto outgoingTransitions = _currentState->getOutgoingTransitions();
        for (auto transition : outgoingTransitions) {
            if (transition->evaluate()) {
                _currentTransition = transition;
                _currentState = transition->getEndState();
                _player->switchAnimation(_currentState->getAnimation());
                _player->play(true);    // TODO handle looping
            }
        }
    }


    _player->update();
}

std::vector<glm::mat4> AnimationController::getBoneMatrices() {
    return _player->getCurrentBoneMatrices();

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
