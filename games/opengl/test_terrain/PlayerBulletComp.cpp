//
// Created by mgrus on 19.03.2025.
//

#include "PlayerBulletComp.h"
#include "EnemyHitManager.h"

PlayerBulletComp::PlayerBulletComp(const std::vector<std::shared_ptr<SceneNode>> &enemyNodes) : _enemyNodes(enemyNodes) {
}

void PlayerBulletComp::update() {
    auto pos = getNode()->transform()->position();
    pos += getNode()->transform()->forward() * ftSeconds * 45.0f;

    lifeTime += ftSeconds;
    if (lifeTime >= maxLifeInSeconds) {
        getNode()->disable();
        lifeTime = 0;
    }

    // Check for collision with enemies
    for (auto e: _enemyNodes) {
        if (!e->isActive()) {
            continue;
        }
        float distance = glm::distance(pos, e->getLocation());
        if (distance < 2) {
            // Instead of disabling directly the node (why would we know to do this?)
            // we should call a better api on the enemy nodes bulletHit component.
            // This is better encapsulation, and we do not forcefully just disable the
            // other node directly.
            auto hitManagers = e->getComponents<EnemyHitManager>();
            if (hitManagers.empty()) {
                throw std::runtime_error("expected at least one EnemyHitManager component in Enemy Scene Node, but none found!");
            }

            for (auto hitManager: hitManagers) {
                hitManager->reportBulletHit(this);
            }

            e->disable();
            auto explosionComp = (SceneNode*) e->getExtraData();
            explosionComp->enable();
        }
    }


}
