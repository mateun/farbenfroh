//
// Created by mgrus on 16.02.2025.
//

#ifndef ANIMATIONTRANSITION_H
#define ANIMATIONTRANSITION_H

#include <vector>

class AnimationController;
class TransitionConstraint;
class AnimationState;

/**
* This class represents the transition between 2 animation states.
* It always has a source and target state and a number of constraints.
*
*/
class AnimationTransition {

public:
    AnimationTransition(AnimationController* animationController, AnimationState* startState, AnimationState* endState);
    ~AnimationTransition();
    void addConstraint(TransitionConstraint* constraint);
    bool evaluate();

    void clearConstraints();

    AnimationState * getEndState();
    AnimationState * getStartState();

private:
    std::vector<TransitionConstraint*> constraints;
    AnimationState * _endState = nullptr;
    AnimationState * _startState = nullptr;
    AnimationController * _animationController = nullptr
;
};

enum class ConstraintOperator {
    EQUAL,
    LESSEQUAL,
    GREATEREQUAL,
    NOT,
};

/**
* Represents a boolean expression
*/
class TransitionConstraint {
public:
    std::string propertyKey;
    AnimationProperty property;
    ConstraintOperator constraintOperator;

};



#endif //ANIMATIONTRANSITION_H
