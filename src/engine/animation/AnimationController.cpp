//
// Created by mgrus on 16.02.2025.
//

#include "AnimationController.h"

AnimationController::AnimationController() {
}

std::optional<AnimationProperty> AnimationController::getProperty(const std::string &key) {
    if (!properties.contains(key)) {
        return std::nullopt;
    }

    return properties[key];

}

void AnimationController::setProperty(const std::string key, AnimationProperty animationProperty) {
    properties[key] = animationProperty;
}
