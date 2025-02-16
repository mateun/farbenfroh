//
// Created by mgrus on 16.02.2025.
//

#ifndef ANIMATIONTRANSITION_H
#define ANIMATIONTRANSITION_H

class AnimationController;
class TransitionConstraint;

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


private:
    std::vector<TransitionConstraint*> constraints;
    AnimationState * _endState = nullptr;
    AnimationState * _startState = nullptr;
    AnimationController * _animationController = nullptr;
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
    std::string key;
    std::string stringValue;
    float floatValue;
    bool boolValue;
    ConstraintOperator constraintOperator;

};



#endif //ANIMATIONTRANSITION_H
