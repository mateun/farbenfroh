//
// Created by mgrus on 02.11.2024.
//

#include "AnimationPlayer.h"
#include "../../graphics.h"

AnimationPlayer::AnimationPlayer(Animation *animation, Mesh* mesh) : animation(animation), mesh(mesh) {

}

void AnimationPlayer::play(bool looped) {
    playing = true;
    animTime = 0;
    this->looped = looped;
}

void AnimationPlayer::stop() {
    playing = false;
}

void AnimationPlayer::switchAnimation(Animation *animation) {
    stop();
    this->animation = animation;
}

void AnimationPlayer::setMesh(Mesh* mesh) {
    stop();
    this->mesh = mesh;
}

int AnimationPlayer::getRotationIndex(const std::string& jointName) {
    auto allSamples = *animation->samplesPerJoint[jointName];
    for (int i = 0; i< allSamples.size()-1; ++i) {
        if (animTime < allSamples[i + 1]->time) {
            return i;
        }

    }
    return -1;
}

void AnimationPlayer::update() {

    static float frameTime = 0;
    if (animation && playing) {
        // animTime += (ftSeconds * 1000.0f);
        // frameTime += (ftSeconds * 1000.0f);

        animTime += ftSeconds;
        if (looped) {
            animTime = fmod(animTime, animation->duration);
        } else {
            if (animTime > (animation->duration)) {
                animTime = 0;
                playing = false;
            }
        }

        //printf("animTime %f\n", animTime);



        std::vector<glm::mat4> boneMatrices;
        for (auto j: mesh->skeleton->joints) {
            if (animation) {
                auto jointSamples = animation->samplesPerJoint[j->name];
                if (jointSamples) {

                    auto fromSampleIndex = getRotationIndex(j->name);
                    //printf("fromSampleIndex: %d\n", fromSampleIndex);
                    if (fromSampleIndex == -1) {
                        exit(8567);
                    }
                    auto toSampleIndex = fromSampleIndex + 1;
                    if (toSampleIndex >= jointSamples->size()) {
                        toSampleIndex = 0;
                    }

                    if (toSampleIndex > fromSampleIndex) {
                        auto fromSample = (*jointSamples)[fromSampleIndex];
                        auto toSample = (*jointSamples)[toSampleIndex];
                        float scaleFactor = ((float) animTime - fromSample->time ) / (float) (toSample->time - fromSample->time);
                        //printf("scalefactor: %f\n", scaleFactor);
                        auto interpTranslation = glm::mix(fromSample->translation, toSample->translation, scaleFactor);
                        auto interpRotation = glm::slerp(fromSample->rotation, toSample->rotation, scaleFactor);

                        j->localTransform = glm::translate(glm::mat4(1), interpTranslation) *
                                            glm::toMat4(interpRotation);
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


/**
* The joint parameter must have its name and inverseBindMatrix set.
* The final transform will be returned as the result.
*/
glm::mat4 AnimationPlayer::calculateFramePoseForJoint(int frame, Joint* joint) {
    auto jointSamples = animation->samplesPerJoint[joint->name];

    if (jointSamples && jointSamples->size() > frame ) {

        auto sample = (*jointSamples)[frame];
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
            auto jointSamples = animation->samplesPerJoint[j->name];
            if (jointSamples &&
                jointSamples->size() > currentFrame) {

                auto sampleIndex = getRotationIndex(j->name);

                auto sample = (*jointSamples)[sampleIndex];
                j->localTransform = glm::translate(glm::mat4(1), sample->translation) *
                                    glm::toMat4(sample->rotation) ;
                j->globalTransform = calculateWorldTransform(j, j->localTransform);
                j->finalTransform = j->globalTransform * j->inverseBindMatrix;

            }
        }
        boneMatrices.push_back(j->finalTransform);
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
