//
// Created by mgrus on 18.02.2025.
//

#include "BoneMatrixCalculator.h"
#include "Pose.h"

int getSampleIndex(Animation* animation, const std::string& jointName, float animationTime) {
    auto allSamples = *animation->samplesPerJoint[jointName];
    for (int i = 0; i< allSamples.size()-1; ++i) {
        if (animationTime < allSamples[i + 1]->time) {
            return i;
        }

    }
    return -1;
}


Pose* BoneMatrixCalculator::calculatePose(Animation *animation, Skeleton *skeleton, float animationTime) {
    Pose* pose = new Pose();

    for (auto j: skeleton->joints) {

            auto jointSamples = animation->samplesPerJoint[j->name];
            if (jointSamples) {

                auto fromSampleIndex = getSampleIndex(animation, j->name, animationTime);
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
                    float scaleFactor = ((float) animationTime - fromSample->time ) / (float) (toSample->time - fromSample->time);
                    //printf("scalefactor: %f\n", scaleFactor);
                    auto interpTranslation = glm::mix(fromSample->translation, toSample->translation, scaleFactor);
                    auto interpRotation = glm::slerp(fromSample->rotation, toSample->rotation, scaleFactor);

                    auto localTransform = glm::translate(glm::mat4(1), interpTranslation) *
                                        glm::toMat4(interpRotation);

                    //j->globalTransform = calculateWorldTransform(j, j->localTransform);
                    //j->finalTransform = j->globalTransform * j->inverseBindMatrix;
                    Joint* newJoint = new Joint();
                    newJoint->name = j->name;
                    newJoint->translation = interpTranslation;
                    newJoint->rotation = interpRotation;
                    newJoint->localTransform = localTransform;
                    pose->joints.push_back(newJoint);
                }
            }
        }
    return pose;
}

Pose* BoneMatrixCalculator::calculateBlendedPose(Pose *pose1, Pose *pose2, Skeleton* skeleton,
    float elapsedTime, float blendDuration) {
    if (elapsedTime < blendDuration) {
        float blendFactor = elapsedTime / blendDuration;
        Pose* blendedPose = new Pose();
        for (int i = 0; i< skeleton->joints.size(); i++) {
            auto blendedJoint = new Joint();
            blendedJoint->translation = mix(pose1->joints[i]->translation, pose2->joints[i]->translation, blendFactor);
            blendedJoint->rotation = slerp(pose1->joints[i]->rotation, pose2->joints[i]->rotation, blendFactor);
            blendedPose->joints.push_back(blendedJoint);

        }
        return blendedPose;
    }
    return nullptr;
}

static glm::mat4 calculateGlobalTransform(Joint* j, glm::mat4 currentTransform) {
    if (j->parent) {
        currentTransform = j->parent->localTransform * currentTransform;
        return calculateGlobalTransform(j->parent, currentTransform);
    }

    return currentTransform;

}

std::vector<glm::mat4> BoneMatrixCalculator::calculateFinalSkinMatrices(Pose* pose) {
    std::vector<glm::mat4> finalMatrices;
    for (auto j: pose->joints) {
        auto globalTransform = calculateGlobalTransform(j, j->localTransform);
        auto finalTransform = j->globalTransform * j->inverseBindMatrix;;
        finalMatrices.push_back(globalTransform);
    }

    return finalMatrices;

}
