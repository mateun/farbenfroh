//
// Created by mgrus on 16.02.2025.
//

#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <graphics.h>
#include <map>
#include <vector>
#include <string>
#include <optional>
#include <glm/glm.hpp>
#include "AnimationProperty.h"
#include "AnimationState.h"

class AnimationPlayer;
/**
* This class is inspired by the way how modern game engines treat
* animations and how they blend together and are chosen for a given gameplay moment.
* We have a state machine with constrained transitions.
* States can be either be pure Animations or AnimationBlends of more than 1 animation.
* Between the states, we have transitions, e.g. from idle to walk.
* Each transition has a constraint which decides if this transition is actually taken.
*/
class AnimationController {

public:
    AnimationController();
    ~AnimationController();
    void addAnimationState(AnimationState* animationState);

    /**
    * This must be called each frame so the animation states can be accurately calculated.
    */
    void update();

    /**
    * This function returns the bone matrices for the current animation state (animation or blendstate).
    */
    std::vector<glm::mat4> getBoneMatrices();

    /**
    * Get and set the properties of this controller.
    */
    std::optional<AnimationProperty> getProperty(const std::string& key);
    void setProperty(const std::string key, AnimationProperty animationProperty);

    AnimationState* getCurrentState();

private:
    std::map<std::string, AnimationProperty> properties;
    AnimationState* _currentState = nullptr;
    std::vector<AnimationState*> _animationStates;
    AnimationPlayer* _player = nullptr;
    Mesh * _mesh = nullptr;
};



#endif //ANIMATIONCONTROLLER_H
