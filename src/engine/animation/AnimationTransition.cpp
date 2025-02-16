//
// Created by mgrus on 16.02.2025.
//

#include "AnimationTransition.h"

AnimationTransition::AnimationTransition(AnimationController *animationController, AnimationState *startState,
    AnimationState *endState) : _animationController(animationController), _startState(startState), _endState(endState) {
}
