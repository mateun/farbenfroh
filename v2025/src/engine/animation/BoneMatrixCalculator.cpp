//
// Created by mgrus on 18.02.2025.
//

#define NOMINMAX
#include <engine/animation/BoneMatrixCalculator.h>
#include <engine/animation/PerBoneBlendData.h>
#include <engine/animation/Pose.h>
#include <engine/animation/Joint.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>


int getSampleIndex(Animation* animation, const std::string& jointName, float animationTime, SampleType sampleType) {
    auto allSamples = animation->findSamples(jointName, sampleType);
    for (int i = 0; i< allSamples.size()-1; ++i) {
        if (animationTime < allSamples[i + 1]->time) {
            return i;
        }

    }
    return -1;
}

Pose* BoneMatrixCalculator::calculateBlendedPose(Animation* first, Animation* second, gru::Skeleton* skeleton, float animTime, float blendWeight, PerBoneBlendData* perBoneBlendData) {
    Pose* blendedPose = new Pose;
    auto pose1 = calculatePose(first, skeleton, animTime);
    auto pose2 = calculatePose(second, skeleton, animTime);
    for (auto j: skeleton->joints) {
        auto j1 = pose1->findJointByName(j->name);
        auto j2 = pose2->findJointByName(j->name);

        // We can implement blendlayer masking by assigning a per-joint blend weight.
        // So we can effictively override for given joints, one animation completely.
        // And we don't need to have special masks, which don't work with blending anyway.
        // TODO somehow apply such a blend-layer-mask to each joint so we can use it here dynamically.
        float perJointBlendWeight = blendWeight;
        if (perBoneBlendData) {
            auto foundWeight = perBoneBlendData->getWeightForBone(j->name);
            if (foundWeight.has_value()) {
                    perJointBlendWeight = foundWeight.value();
            }
        }


        auto interpolatedRotation = glm::slerp(j1->currentPoseOrientation[first->name], j2->currentPoseOrientation[second->name], perJointBlendWeight);
        auto interpolatedLocation = glm::mix(j1->currentPoseLocation[first->name], j2->currentPoseLocation[second->name], blendWeight);

        j->currentPoseLocalTransform= translate(glm::mat4(1), interpolatedLocation) *
                               toMat4(interpolatedRotation);
        blendedPose->joints.push_back(j);
    }

    return blendedPose;
}



Pose *BoneMatrixCalculator::calculatePose(Animation *animation, gru::Skeleton *skeleton, float animTime) {
    Pose* pose = new Pose();

    for (auto j: skeleton->joints) {
        if (animation->isJointMasked(j)) {
            j->currentPoseLocalTransform= j->bindPoseLocalTransform;
            j->currentPoseLocation[animation->name] = j->translation;
            j->currentPoseOrientation[animation->name] = j->rotation;
            pose->joints.push_back(j);
            continue;
        }
        auto rotationSamples = animation->findSamples(j->name, SampleType::rotation);
        auto translationSamples = animation->findSamples(j->name, SampleType::translation);
        glm::quat interpolatedRotation;
        auto interpolatedTranslation = glm::vec3(0);

        if (!rotationSamples.empty()) {
            auto fromRotationSampleIndex = getSampleIndex(animation, j->name, animTime, SampleType::rotation);
            auto toSampleIndex = fromRotationSampleIndex + 1;
            if (toSampleIndex >= rotationSamples.size()) {
                toSampleIndex = 0;
            }

            auto fromRotationSample = rotationSamples[fromRotationSampleIndex];
            auto toRotationSample = rotationSamples[toSampleIndex];
            float rotationBlendFactor = ((float) animTime - fromRotationSample->time ) / (float) (toRotationSample->time - fromRotationSample->time);
            interpolatedRotation = glm::slerp(fromRotationSample->rotation, toRotationSample->rotation, rotationBlendFactor);
        }

        if (!translationSamples.empty()) {
            auto fromTranslationSampleIndex = getSampleIndex(animation, j->name, animTime, SampleType::translation);
            auto toTranslationSampleIndex = fromTranslationSampleIndex + 1;
            if (toTranslationSampleIndex >= translationSamples.size()) {
                toTranslationSampleIndex = 0;
            }
            auto fromTranslationSample = translationSamples[fromTranslationSampleIndex];
            auto toTranslationSample = translationSamples[toTranslationSampleIndex];
            float translationBlendFactor = ((float) animTime - fromTranslationSample->time ) / (float) (toTranslationSample->time - fromTranslationSample->time);
            interpolatedTranslation = mix(fromTranslationSample->translation, toTranslationSample->translation, translationBlendFactor);

        }

        j->currentPoseLocalTransform= translate(glm::mat4(1), interpolatedTranslation) *
                                toMat4(interpolatedRotation);
        j->currentPoseLocation[animation->name] = interpolatedTranslation;
        j->currentPoseOrientation[animation->name] = interpolatedRotation;
        //j->currentPoseGlobalTransform = calculateWorldTransform(j, j->currentPoseLocalTransform);
        //j->finalTransform = j->globalTransform * j->inverseBindMatrix;

        // Joint* newJoint = new Joint();
        // newJoint->name = j->name;
        // newJoint->translation = interpolatedTranslation;
        // newJoint->rotation = interpolatedRotation;
        // newJoint->currentPoseLocalTransform = localTransform;
        pose->joints.push_back(j);

    }
    return pose;
}


/*
Pose* BoneMatrixCalculator::calculatePose_(Animation *animation, Skeleton *skeleton, float animationTime) {
    Pose* pose = new Pose();

    for (auto j: skeleton->joints) {

            // TODO adapt to separate rotation and translation samples!! See <class>AnimationPlayer</class>!
            auto jointSamples = animation->findSamples(j->name, SampleType::rotation);
            if (!jointSamples.empty()) {

                auto fromSampleIndex = getRotationSampleIndex(animation, j->name, animationTime);
                //printf("fromSampleIndex: %d\n", fromSampleIndex);
                if (fromSampleIndex == -1) {
                    exit(8567);
                }
                auto toSampleIndex = fromSampleIndex + 1;
                if (toSampleIndex >= jointSamples.size()) {
                    toSampleIndex = 0;
                }

                if (toSampleIndex > fromSampleIndex) {
                    auto fromSample = jointSamples[fromSampleIndex];
                    auto toSample = jointSamples[toSampleIndex];
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
*/

Pose* BoneMatrixCalculator::calculateBlendedPose(Pose *pose1, Pose *pose2, gru::Skeleton* skeleton,
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

glm::mat4 BoneMatrixCalculator::calculateGlobalTransform(Joint* j, glm::mat4 currentTransform) {
    if (j->parent) {
        currentTransform = j->parent->currentPoseLocalTransform * currentTransform;
        return calculateGlobalTransform(j->parent, currentTransform);
    }

    return currentTransform;

}

std::vector<glm::mat4> BoneMatrixCalculator::calculateGlobalMatrices(Pose *pose) {
    std::vector<glm::mat4> matrices;
    for (auto j: pose->joints) {
        auto globalTransform = calculateGlobalTransform(j, j->currentPoseLocalTransform);
        matrices.push_back(globalTransform);
    }
    return matrices;
}

std::vector<glm::mat4> BoneMatrixCalculator::calculateFinalSkinMatrices(Pose* pose) {
    std::vector<glm::mat4> finalMatrices;
    for (auto j: pose->joints) {
        auto globalTransform = calculateGlobalTransform(j, j->currentPoseLocalTransform);
        auto finalTransform = globalTransform * j->inverseBindMatrix;;
        finalMatrices.push_back(finalTransform);
    }

    return finalMatrices;

}
