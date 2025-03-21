//
// Created by mgrus on 19.03.2025.
//

#include "PlayerBulletComp.h"
#include "EnemyHitManager.h"

PlayerBulletComp::PlayerBulletComp() {
}

void PlayerBulletComp::update() {
    auto pos = getNode().lock()->getLocation();
    pos += getNode().lock()->transform()->forward() * ftSeconds * 45.0f;
    getNode().lock()->setLocation(pos);

    lifeTime += ftSeconds;
    if (lifeTime >= maxLifeInSeconds) {
        getNode().lock()->disable();
        lifeTime = 0;
    }

    // TODO where and how to check for collisions?
    // In separate generic "physics" or collision engine?!
    // Check for collision with enemies
    // for (auto e: _enemyNodes) {
    //     if (!e->isActive()) {
    //         continue;
    //     }
    //     float distance = glm::distance(pos, e->getLocation());
    //     if (distance < 2) {
    //         // Instead of disabling directly the node (why would we know to do this?)
    //         // we should call a better api on the enemy nodes bulletHit component.
    //         // This is better encapsulation, and we do not forcefully just disable the
    //         // other node directly.
    //         auto hitManagers = e->getComponents<EnemyHitManager>();
    //         if (hitManagers.empty()) {
    //             throw std::runtime_error("expected at least one EnemyHitManager component in Enemy Scene Node, but none found!");
    //         }
    //
    //         for (auto hitManager: hitManagers) {
    //             hitManager->reportBulletHit(this);
    //         }
    //
    //         e->disable();
    //         auto explosionComp = (SceneNode*) e->getExtraData();
    //         explosionComp->enable();
    //     }
    // }


}
