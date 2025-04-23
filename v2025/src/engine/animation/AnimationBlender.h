//
// Created by mgrus on 16.02.2025.
//

#ifndef ANIMATIONBLENDER_H
#define ANIMATIONBLENDER_H

class Animation;

class PerBoneBlendData;
/**
* Allows for the blending of 2 animations.
*/
class AnimationBlender {

public:
    AnimationBlender(Animation* first, Animation* second, PerBoneBlendData* perBoneBlendData);
    Animation* first();
    Animation* second();
    PerBoneBlendData* perBoneBlendData();

private:
    Animation * _first = nullptr;
    Animation * _second = nullptr;
    PerBoneBlendData * _perBoneBlendData = nullptr;
};



#endif //ANIMATIONBLENDER_H
