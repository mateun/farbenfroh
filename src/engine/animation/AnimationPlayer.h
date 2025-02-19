//
// Created by mgrus on 02.11.2024.
//

#ifndef SIMPLE_KING_ANIMATIONPLAYER_H
#define SIMPLE_KING_ANIMATIONPLAYER_H

#include "../../graphics.h"

Animation* findAnimationByName(std::string name, Mesh* mesh);

class AnimationPlayer {
public:
    AnimationPlayer(Animation* animation, Mesh* mesh);
    void play(bool looped);
    // This updates the current pose matrices based on frame time interpolation
    void update();

    glm::mat4 calculateFramePoseForJoint(int frame, Joint *joint);
    glm::mat4 calculateInterpolatedFramePose(Joint* joint);
    glm::mat4 calculateInterpolatedGlobalMatrixForJoint(Joint* j);

    // This updates the current pose for a specific explicit frame, without interpolation
    void calculateFramePose(int frame);
    void stop();
    void switchAnimation(Animation* animation);
    void setMesh(Mesh* mesh);

    // Searches for the index of the last rotation sample based on the given time
    int getRotationIndexForTime(const std::string& jointName, float time);

    // Searches for the index of the last translation sample based on the given time
    int getTranslationIndexForTime(const std::string &jointName, float time);

    int getTranslationIndex();

    std::vector<glm::mat4> getCurrentBoneMatrices();

private:
    Animation* animation= nullptr;
    Mesh *mesh = nullptr;

    bool playing = false;
    bool looped = false;
    double currentFrame = 0;
    float animTime = 0;
    std::vector<glm::mat4> boneMatrices;

};


#endif //SIMPLE_KING_ANIMATIONPLAYER_H
