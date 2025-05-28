//
// Created by mgrus on 28.05.2025.
//

#include "../animation/include/property_animation.h"
#include "iostream"


void PropertyAnimation::update(float delta) {
    for (auto at : tracks) {
        at->eval(delta);
    }
}
