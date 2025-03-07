//
// Created by mgrus on 16.02.2025.
//

#ifndef ANIMATIONBLENDER_H
#define ANIMATIONBLENDER_H



/**
* Allows for the blending of n animations based on some criteria.
*/
class AnimationBlender {

public:
    AnimationBlender();
    void addAnimation(Animation* animation, const std::string& layerFilter);
    void removeAnimation(Animation* animation);

private:
    std::vector<Animation*> animations;


};



#endif //ANIMATIONBLENDER_H
