//
// Created by mgrus on 19.03.2025.
//

#include "PlayerBulletComp.h"

void PlayerBulletComp::update() {
    auto pos = getTransform().position;
    pos += getTransform().forward * ftSeconds * 45.0f;
    b->setLocation(pos);
    BulletData* bd = (BulletData*) b->getExtraData();
    bd->currentLifeInSeconds += ftSeconds;
    if (bd->currentLifeInSeconds >= bd->maxLifeInSeconds) {
        b->disable();
        bd->currentLifeInSeconds = 0;
    }

    // Check for collision with enemies
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
