//
// Created by mgrus on 16.02.2025.
//

#include "AnimationState.h"

AnimationState::AnimationState(Animation *animation, const std::string &name) : _animation(animation), _name(name) {
}

AnimationState::AnimationState(AnimationBlender *blender, const std::string &name) : _blender(blender), _name(name) {
}

AnimationState::~AnimationState() {
}

void AnimationState::addOutgoingTransition(AnimationTransition *animationTransition) {
    _outgoingTransitions.push_back(animationTransition);
}

void AnimationState::addIncomingTransition(AnimationTransition *animationTransition) {
    _incomingTransitions.push_back(animationTransition);
}

std::vector<AnimationTransition *> AnimationState::getOutgoingTransitions() {
    return _outgoingTransitions;
}

std::vector<AnimationTransition *> AnimationState::getIncomingTransitions() {
    return _incomingTransitions;
}

Animation * AnimationState::getAnimation() {
    return _animation;
}

AnimationBlender * AnimationState::getBlender() {
    return _blender;
}
