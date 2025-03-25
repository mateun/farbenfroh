//
// Created by mgrus on 02.11.2024.
//

#include "AnimationPlayer.h"
#include "Animation.h"
#include <engine/animation/Joint.h>
#include <engine/animation/BoneMatrixCalculator.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "AnimationBlender.h"
#include <engine/graphics/Mesh.h>
#include "../../graphics.h"

extern float ftSeconds;

AnimationPlayer::AnimationPlayer() {
}


AnimationPlayer::AnimationPlayer(Animation *animation, Mesh* mesh) : animation(animation), mesh(mesh) {

}

AnimationPlayer::AnimationPlayer(AnimationBlender *animBlender, Mesh *mesh) : animationBlender(animBlender), mesh(mesh) {
}

void AnimationPlayer::play(bool looped) {
    playing = true;
    animTime = 0;
    this->looped = looped;
}

void AnimationPlayer::stop() {
    playing = false;
    looped = false;
}

void AnimationPlayer::switchAnimation(Animation *animation) {
    stop();
    this->animation = animation;
    this->animTime = 0;
    this->animationBlender = nullptr;

}

void AnimationPlayer::setAnimationBlender(AnimationBlender *animBlender) {
    this->animationBlender = animBlender;
    this->animation = nullptr;
}

void AnimationPlayer::setMesh(Mesh* mesh) {
    stop();

    this->mesh = mesh;
}

int AnimationPlayer::getRotationIndexForTime(const std::string& jointName, float time) {
    auto allRotationSamples = animation->findSamples(jointName, SampleType::rotation);
    for (int i = 0; i< allRotationSamples.size()-1; ++i) {
        if (time < allRotationSamples[i + 1]->time) {
            return i;
        }
    }
    return -1;
}

int AnimationPlayer::getTranslationIndexForTime(const std::string& jointName, float time) {
    auto allSamples = animation->findSamples(jointName, SampleType::translation);
    for (int i = 0; i< allSamples.size()-1; ++i) {
        if (animTime < allSamples[i + 1]->time) {
            return i;
        }
    }
    return -1;
}

std::vector<glm::mat4> AnimationPlayer::getCurrentBoneMatrices() {
    return boneMatrices;
}

glm::mat4 AnimationPlayer::calculateInterpolatedGlobalMatrixForJoint(Joint* j) {
    if (!animation) {
        return glm::mat4();
    }
    auto rotationSamples = animation->findSamples(j->name, SampleType::rotation);
    auto translationSamples = animation->findSamples(j->name, SampleType::translation);
    glm::quat interpolatedRotation;
    auto interpolatedTranslation = glm::vec3(0);

    if (!rotationSamples.empty()) {
        auto fromRotationSampleIndex = getRotationIndexForTime(j->name, animTime);
        auto toSampleIndex = fromRotationSampleIndex + 1;
        if (toSampleIndex >= rotationSamples.size()) {
            toSampleIndex = 0;
        }

        auto fromRotationSample = rotationSamples[fromRotationSampleIndex];
        auto toRotationSample = rotationSamples[toSampleIndex];
        float rotationBlendFactor = ((float) animTime - fromRotationSample->time ) / (float) (toRotationSample->time - fromRotationSample->time);
        interpolatedRotation = slerp(fromRotationSample->rotation, toRotationSample->rotation, rotationBlendFactor);
    }

    if (!translationSamples.empty()) {
        auto fromTranslationSampleIndex = getTranslationIndexForTime(j->name, animTime);
        auto toTranslationSampleIndex = fromTranslationSampleIndex + 1;
        if (toTranslationSampleIndex >= translationSamples.size()) {
            toTranslationSampleIndex = 0;
        }
        auto fromTranslationSample = translationSamples[fromTranslationSampleIndex];
        auto toTranslationSample = translationSamples[toTranslationSampleIndex];
        float translationBlendFactor = ((float) animTime - fromTranslationSample->time ) / (float) (toTranslationSample->time - fromTranslationSample->time);
        interpolatedTranslation = mix(fromTranslationSample->translation, toTranslationSample->translation, translationBlendFactor);

    }

    j->currentPoseLocalTransform = translate(glm::mat4(1), interpolatedTranslation) *
                            toMat4(interpolatedRotation);
    j->currentPoseGlobalTransform = calculateWorldTransform(j, j->currentPoseLocalTransform);

    return j->currentPoseGlobalTransform;
}

void AnimationPlayer::update() {

    static float frameTime = 0;
    if (animation || animationBlender && playing) {

        animTime += ftSeconds;

        if (looped) {
            if (animation) {
                animTime = fmod(animTime, animation->duration);
            } else if (animationBlender) {
                // Take the shorter one of the 2 animations, sometimes they are minimally off, but to avoid
                // sampling issues in the shorter one, we take its duration, so we will always find a sample
                //, also in the longer one.
                float duration = std::min(animationBlender->first()->duration, animationBlender->second()->duration);
                animTime = fmod(animTime, duration);
            }

        } else {
            if (animation && (animTime > (animation->duration))) {
                animTime = 0;
                playing = false;
            }
            else if (animationBlender && (animTime > (std::min(animationBlender->first()->duration, animationBlender->second()->duration)))) {
                animTime = 0;
                playing = false;
            }
        }

        boneMatrices.clear();
        if (animation) {
            auto finalPose = BoneMatrixCalculator().calculatePose(animation, mesh->skeleton, animTime);
            boneMatrices = BoneMatrixCalculator().calculateFinalSkinMatrices(finalPose);
        }
        if (animationBlender) {
            auto finalPose = BoneMatrixCalculator().calculateBlendedPose(animationBlender->first(), animationBlender->second(), mesh->skeleton, animTime, 0.5, animationBlender->perBoneBlendData());
            boneMatrices = BoneMatrixCalculator().calculateFinalSkinMatrices(finalPose);
        }

        // for (auto j: mesh->skeleton->joints) {
        //     if (animation) {
        //         j->currentPoseGlobalTransform = calculateInterpolatedGlobalMatrixForJoint(j);
        //         j->currentPoseFinalTransform = j->currentPoseGlobalTransform * j->inverseBindMatrix;
        //     }
        //     boneMatrices.push_back(j->currentPoseFinalTransform);
        // }
    }
}

glm::mat4 AnimationPlayer::calculateInterpolatedFramePose(Joint* joint) {
    calculateInterpolatedGlobalMatrixForJoint(joint);
    joint->currentPoseFinalTransform = joint->currentPoseGlobalTransform * joint->inverseBindMatrix;
    return joint->currentPoseFinalTransform;
}


/**
* The joint parameter must have its name and inverseBindMatrix set.
* The final transform will be returned as the result.
*/
glm::mat4 AnimationPlayer::calculateFramePoseForJoint(int frame, Joint* joint) {
    auto rotationJointSamples = animation->findSamples(joint->name, SampleType::rotation);
    auto translationJointSamples = animation->findSamples(joint->name, SampleType::translation);

    if (rotationJointSamples.size() > frame ) {

        auto sample = rotationJointSamples[frame];
        auto localTransform = glm::translate(glm::mat4(1), sample->translation) *
                            glm::toMat4(sample->rotation) ;
        auto globalTransform  = calculateWorldTransform(joint, localTransform);

        // We do NOT multiply the inverse bind matrix here. This is only needed for the skinned vertices.
        auto finalTransform = globalTransform;
        return finalTransform;
    }

    return glm::mat4(1);


}

void AnimationPlayer::calculateFramePose(int frame) {
    std::vector<glm::mat4> boneMatrices;
    for (auto j: mesh->skeleton->joints) {
        if (animation) {
            auto rotationJointSamples = animation->findSamples(j->name, SampleType::rotation);
            if (
                rotationJointSamples.size() > currentFrame) {

                auto sampleIndex = getRotationIndexForTime(j->name, animTime);

                auto sample = rotationJointSamples[sampleIndex];
                j->currentPoseLocalTransform = glm::translate(glm::mat4(1), sample->translation) *
                                    glm::toMat4(sample->rotation) ;
                j->currentPoseGlobalTransform = calculateWorldTransform(j, j->currentPoseLocalTransform);
                j->currentPoseFinalTransform = j->currentPoseGlobalTransform * j->inverseBindMatrix;

            }
        }
        boneMatrices.push_back(j->currentPoseFinalTransform);
    }
    setBoneMatrices(boneMatrices);

}

Animation* findAnimationByName(std::string name, Mesh* mesh) {
    for (auto a: mesh->animations) {
        if (a->name == name) {
            return a;
        }
    }
    return nullptr;
}

