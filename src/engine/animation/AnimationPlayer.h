//
// Created by mgrus on 02.11.2024.
//

#ifndef SIMPLE_KING_ANIMATIONPLAYER_H
#define SIMPLE_KING_ANIMATIONPLAYER_H

#include "../../graphics.h"

class AnimationPlayer {
public:
    AnimationPlayer(Animation* animation, Mesh* mesh);
    void play(bool looped);
    void update();
    void stop();

private:
    Animation* animation= nullptr;
    Mesh *mesh = nullptr;

    bool playing = false;
    bool looped = false;
    int currentFrame = 0;


};


#endif //SIMPLE_KING_ANIMATIONPLAYER_H
