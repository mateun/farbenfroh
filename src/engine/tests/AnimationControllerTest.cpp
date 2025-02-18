//
// Created by mgrus on 17.02.2025.
//
#include <Windows.h>
#include <stdexcept>
#include <engine/game/default_app.h>

#include "../../graphics.h"
#include "../src/engine/animation/AnimationController.h"
#include "../src/engine/animation/AnimationTransition.h"
#include "../src/engine/animation/AnimationProperty.h"
#include "../src/engine/animation/AnimationState.h"
#include "../src/engine/animation/Animation.h"


class AnimationTest : public DefaultApp {
    void init() override;
    void update() override {};
    void render() override {};
    std::vector<std::string> getAssetFolder() override {
        return {"../src/engine/tests/assets/"};
    };
    bool shouldAutoImportAssets() override {
        return true;
    };

};


DefaultApp* getGame() {
    return new AnimationTest();
}

void AnimationTest::init() {

    DefaultApp::init();
    // Properties test
    Mesh* mesh = getMeshByName("test_skeletal_mesh");
    auto animationController = new AnimationController(mesh);
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
    AnimationState idleState(&animation, "idle");
    AnimationState walkState(&animation, "walk");
    animationController->addAnimationState(&idleState);
    animationController->addAnimationState(&walkState);

    AnimationTransition transition(animationController, &idleState, &walkState );
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

    // Test transition
    animationController->update();
    if (animationController->getCurrentState() != &walkState) {
        throw std::runtime_error("Failed updating idle-walk transition");
    }

    // TODO test actual bone matrices
    auto boneMatrices = animationController->getBoneMatrices();
    if (boneMatrices.empty()) {
        throw std::runtime_error("Failed getting bone matrices");
    }


    // Exit with zero, means succesful test.
    exit(0);

}
