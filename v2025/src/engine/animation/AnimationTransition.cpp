//
// Created by mgrus on 16.02.2025.
//

#include "AnimationTransition.h"

#include <engine/game/Timing.h>

#include "AnimationProperty.h"
#include "AnimationState.h"
#include "AnimationController.h"

AnimationTransition::AnimationTransition(AnimationController *animationController, AnimationState *startState,
    AnimationState *endState) : _animationController(animationController), _startState(startState), _endState(endState) {
    startState->addOutgoingTransition(this);
    endState->addIncomingTransition(this);
}

AnimationTransition::~AnimationTransition() {
}

void AnimationTransition::addConstraint(TransitionConstraint *constraint) {
    constraints.push_back(constraint);
}

bool AnimationTransition::evaluate() {
    for (auto constraint : constraints) {
        auto propertyOptional = _animationController->getProperty(constraint->propertyKey);
        if (!propertyOptional.has_value()) {
            return false;
        }

        auto property = propertyOptional.value();

        if (constraint->constraintOperator == ConstraintOperator::EQUAL) {
            if (property != constraint->property) {
                return false;
            }
        }

        if (constraint->constraintOperator == ConstraintOperator::GREATEREQUAL) {
            if (!(property >= constraint->property)) {
                return false;
            }
        }
        if (constraint->constraintOperator == ConstraintOperator::LESSEQUAL) {
            if (!(property <= constraint->property)) {
                return false;
            }
        }
        if (constraint->constraintOperator == ConstraintOperator::NOT) {
            if (!(property != constraint->property)) {
                return false;
            }
        }

    }

    // We survived all checks:
    return true;
}

void AnimationTransition::update() {
    if (_isActive) {
        _elapsedTime += Timing::lastFrameTimeInSeconds();
        if (_elapsedTime < _durationInSeconds) {

        }
    }
}

void AnimationTransition::clearConstraints() {
    constraints.clear();
}

void AnimationTransition::activate() {
    _isActive = true;
}

AnimationState * AnimationTransition::getEndState() {
    return _endState;
}

AnimationState * AnimationTransition::getStartState() {
    return _startState;
}





