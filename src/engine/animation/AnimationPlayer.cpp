//
// Created by mgrus on 02.11.2024.
//

#include "AnimationPlayer.h"
#include "../../graphics.h"

AnimationPlayer::AnimationPlayer(Animation *animation, Mesh* mesh) : animation(animation), mesh(mesh) {

}

void AnimationPlayer::play(bool looped) {
    playing = true;
    this->looped = looped;
}

void AnimationPlayer::stop() {
    playing = false;
}

void AnimationPlayer::update() {
    static float animTime = 0;
    static float frameTime = 0;
    if (animation && playing) {
        animTime += (ftSeconds * 1000.0f);
        frameTime += (ftSeconds * 1000.0f);

        // TODO: Interpolate the key frames according to the current time
        // in between the two frames we are.
        // Currently we just update at 30fps
        if (frameTime > 33.3333) {
            currentFrame++;
            frameTime = 0;
        }

        if (currentFrame > (animation->frames - 1)) {
            currentFrame = 0;
            animTime = 0;
            if (!looped) {
                playing = false;
            }
        }

        std::vector<glm::mat4> boneMatrices;
        for (auto j: mesh->skeleton->joints) {
            if (animation) {
                if (animation->samplesPerJoint[j->name] != nullptr &&
                    (animation->samplesPerJoint[j->name]->size() > currentFrame)) {
                    auto sample = (*animation->samplesPerJoint[j->name])[currentFrame];
                    if (sample) {
                        j->localTransform = glm::translate(glm::mat4(1), sample->translation) *
                                            glm::toMat4(sample->rotation);
                        j->globalTransform = calculateWorldTransform(j, j->localTransform);
                        j->finalTransform = j->globalTransform * j->inverseBindMatrix;
                    }
                }

            }
            boneMatrices.push_back(j->finalTransform);
        }
        setBoneMatrices(boneMatrices);
    }
}
