//
// Created by mgrus on 16.02.2025.
//

#ifndef ANIMATIONSTATE_H
#define ANIMATIONSTATE_H


class Animation;
class AnimationBlender;
class AnimationTransition;

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

    void addOutgoingTransition(AnimationTransition* animationTransition);
    void addIncomingTransition(AnimationTransition* animationTransition);

    std::vector<AnimationTransition*> getOutgoingTransitions();
    std::vector<AnimationTransition*> getIncomingTransitions();

    Animation* getAnimation();
    AnimationBlender* getBlender();

private:
    std::vector<AnimationTransition*> _outgoingTransitions;
    std::vector<AnimationTransition*> _incomingTransitions;
    std::string _name;
    Animation * _animation = nullptr;
    AnimationBlender * _blender = nullptr;
};



#endif //ANIMATIONSTATE_H
