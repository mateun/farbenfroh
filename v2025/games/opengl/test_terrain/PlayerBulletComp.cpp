//
// Created by mgrus on 19.03.2025.
//

#include "PlayerBulletComp.h"
#include "EnemyHitManager.h"

PlayerBulletComp::PlayerBulletComp() {
}

void PlayerBulletComp::update() {
    auto pos = getNode().lock()->getLocation();
    pos += getNode().lock()->getForwardVector() * ftSeconds * 45.0f;
    getNode().lock()->setLocation(pos);

    lifeTime += ftSeconds;
    if (lifeTime >= maxLifeInSeconds) {
        getNode().lock()->disable();
        lifeTime = 0;
    }




}
