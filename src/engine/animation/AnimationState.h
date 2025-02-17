//
// Created by mgrus on 16.02.2025.
//

#ifndef ANIMATIONSTATE_H
#define ANIMATIONSTATE_H


class Animation;
class AnimationBlender;

/**
* Represents the nodes in the animation state machine.
* E.g. the State is called "idle", another one is "moving".
* Idle might have a single animation and moving is actually a BlendState
* which blends walk and run based on movement speed.
*/
class AnimationState {

  public:
    AnimationState(Animation* animation, const std::string& name);
    AnimationState(AnimationBlender* blender, const std::string& name);
    ~AnimationState();


};



#endif //ANIMATIONSTATE_H
