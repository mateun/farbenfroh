//
// Created by mgrus on 19.03.2025.
//

#include "PlayerBulletComp.h"

void PlayerBulletComp::update() {
    auto pos = getTransform().worldPosition();
    pos += getTransform().localForward() * ftSeconds * 45.0f;


    lifeTime += ftSeconds;
    if (lifeTime >= maxLifeInSeconds) {

        //b->disable();
        //bd->currentLifeInSeconds = 0;
    }

    // Check for collision with enemies
    // TODO how to get an object such as the enemy list?
    // How to "pass" this to the script compoenent?
    // Or allow the script component to get it somehow.
    for (auto e: enemyList) {
        if (!e->isActive()) {
            continue;
        }
        float distance = glm::distance(pos, e->getLocation());
        if (distance < 2) {
            b->disable();
            e->disable();
            auto explosionComp = (SceneNode*) e->getExtraData();
            explosionComp->enable();
        }
    }


}
