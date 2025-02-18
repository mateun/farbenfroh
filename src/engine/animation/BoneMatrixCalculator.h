//
// Created by mgrus on 18.02.2025.
//

#ifndef BONEMATRIXCALCULATOR_H
#define BONEMATRIXCALCULATOR_H
#include <vector>
#include <glm/glm.hpp>

class Animation;
class Pose;
class Skeleton;


class BoneMatrixCalculator {

public:
    /**
    * It returns a pose, which is a list of joints with each joint having a location, rotation and scaled based
    * on the temporal interpolation between two keyframes.
    */
    Pose* calculatePose(Animation* animation, Skeleton* skeleton, float animationTime);

    /**
    * Does a blend between the two animation poses, e.g. idle and walk.
    * Based on the animatin time, two layers of interpolations are done:
    * 1. The temporal interpolation between the keyframes within each animation (suppose we almost always need to interp., will never hit an exact keyframe)
    * 2. The interpolation between the two animations. We take the local positions from step1 and do the interpolations between the 2 animations, for each bone.
    */
    Pose* calculateBlendedPose(Pose* pose1, Pose* pose2, Skeleton* skeleton, float elapsedTime, float blendDuration);

    /**
    * Calculates the skinning matrices for each joint of the given pose:
    * final = global * inverseBind
    * These matrices can be used in the vertex shader.
    */
    std::vector<glm::mat4> calculateFinalSkinMatrices(Pose* pose);

};



#endif //BONEMATRIXCALCULATOR_H
