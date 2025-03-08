//
// Created by mgrus on 16.02.2025.
//

#include "AnimationBlender.h"

AnimationBlender::AnimationBlender(Animation *first, Animation *second, PerBoneBlendData *perBoneBlendData) : _first(first), _second(second), _perBoneBlendData(perBoneBlendData) {
}

Animation * AnimationBlender::first() {
    return _first;
}

Animation * AnimationBlender::second() {
    return _second;
}

PerBoneBlendData * AnimationBlender::perBoneBlendData() {
    return _perBoneBlendData;
}
