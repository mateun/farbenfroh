//
// Created by mgrus on 16.02.2025.
//

#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <map>
#include <string>
#include <optional>
#include "AnimationProperty.h"



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
	std::optional<AnimationProperty> getProperty(const std::string& key);
    void setProperty(const std::string key, AnimationProperty animationProperty);

private:
  std::map<std::string, AnimationProperty> properties;

};



#endif //ANIMATIONCONTROLLER_H
