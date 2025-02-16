//
// Created by mgrus on 16.02.2025.
//

#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H


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





};



#endif //ANIMATIONCONTROLLER_H
