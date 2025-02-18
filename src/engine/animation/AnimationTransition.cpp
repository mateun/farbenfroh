//
// Created by mgrus on 16.02.2025.
//

#include "AnimationTransition.h"
#include "AnimationProperty.h"

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
            if (!(property == constraint->property)) {
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
        _elapsedTime += ftSeconds;
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

bool AnimationProperty::operator==(const AnimationProperty &other) const {
    if (other.propertyType == PropertyType::BOOL) {
        return other.boolValue == boolValue;
    }
    if (other.propertyType == PropertyType::INT) {
        return other.intValue == intValue;
    }
    if (other.propertyType == PropertyType::FLOAT) {
        return other.floatValue == floatValue;
    }
    if (other.propertyType == PropertyType::STRING) {
        return other.stringValue == stringValue;
    }
}

bool AnimationProperty::operator>=(const AnimationProperty &other) const {
    if (other.propertyType == PropertyType::BOOL) {
        return other.boolValue >= boolValue;
    }
    if (other.propertyType == PropertyType::INT) {
        return intValue >= other.intValue;
    }
    if (other.propertyType == PropertyType::FLOAT) {
        return floatValue >= other.floatValue;
    }
    if (other.propertyType == PropertyType::STRING) {
        return stringValue >= other.stringValue;
    }
}

bool AnimationProperty::operator<=(const AnimationProperty &other) const {
    if (other.propertyType == PropertyType::BOOL) {
        return boolValue <= other.boolValue;
    }
    if (other.propertyType == PropertyType::INT) {
        return intValue <= other.intValue;
    }
    if (other.propertyType == PropertyType::FLOAT) {
        return floatValue <= other.floatValue;
    }
    if (other.propertyType == PropertyType::STRING) {
        return stringValue <= other.stringValue;
    }
}





