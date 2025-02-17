//
// Created by mgrus on 17.02.2025.
//
#include <stdexcept>

#include "AnimationController.h"
#include "AnimationTransition.h"
#include "AnimationProperty.h"
#include "AnimationState.h"
#include "Animation.h"

int main(int argc, const char * argv[]) {

    // Properties test
    auto animationController = new AnimationController();
    auto fooProp = animationController->getProperty("foo");
    if (fooProp.has_value()) {
        throw std::runtime_error("foo property exists but should not");
    }

    AnimationProperty actualFooProp;
    actualFooProp.propertyType = PropertyType::STRING;
    actualFooProp.stringValue = "bar";


    animationController->setProperty("foo", actualFooProp);
    fooProp = animationController->getProperty("foo");
    if (!fooProp.has_value()) {
        throw std::runtime_error("foo property does not exist");
    }
    if (fooProp.value().stringValue != "bar") {
        throw std::runtime_error("foo property does not have 'bar' value");
    }

    // Constraint test string equal
    TransitionConstraint constraint;
    constraint.property = fooProp.value();
    constraint.constraintOperator = ConstraintOperator::EQUAL;
    constraint.propertyKey = "foo";

    Animation animation;
    AnimationState sourceState(&animation, "idle");
    AnimationState targetState(&animation, "walk");

    AnimationTransition transition(animationController, &sourceState, &targetState );
    transition.addConstraint(&constraint);
    auto result = transition.evaluate();
    if (!result) {
        throw new std::runtime_error("Failed evaluating idle-walk transition");
    }

    // Constraint test int GREATEREQUAL
    TransitionConstraint constraint2;
    constraint2.property = {};
    constraint2.constraintOperator = ConstraintOperator::GREATEREQUAL;
    constraint2.property.propertyType = PropertyType::INT;
    constraint2.propertyKey = "myint";
    constraint2.property.intValue = 100;

    AnimationProperty intProp;
    intProp.intValue = 101;
    intProp.propertyType = PropertyType::INT;
    transition.clearConstraints();
    transition.addConstraint(&constraint2);
    animationController->setProperty("myint", intProp);
    result = transition.evaluate();
    if (!result) {
        throw new std::runtime_error("Failed evaluating idle-walk transition");
    }



}
