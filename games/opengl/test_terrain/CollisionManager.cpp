//
// Created by mgrus on 21.03.2025.
//

#include "CollisionManager.h"
#include "PlayerBulletPool.h"


void CollisionManager::handlePlayerBulletsVsEnemies(PlayerBulletPool *playerBulletPool,
    const std::vector<std::shared_ptr<SceneNode>> &enemies) {


    std::vector<SceneNode*> activeEnemies;
    for (auto e : enemies) {
        if (e->isActive()) {
            activeEnemies.push_back(e.get());
        }
    }
    //Check for collision with enemies
    for (auto pb : playerBulletPool->findAllActive()) {
        for (auto e: activeEnemies) {

            float distance = glm::distance(pb->getLocation(), e->getLocation());
            if (distance < 2) {
                // TODO:
                // Instead of disabling directly the node (why would we know to do this?)
                // we should call a better api on the enemy nodes bulletHit component.
                // This is better encapsulation, and we do not forcefully just disable the
                // other node directly.
                e->disable();
                auto explosionParticleSystems = e->findChildren(SceneNodeType::ParticleSystem);
                for (auto ps : explosionParticleSystems) {
                    ps->enable();
                }

            }
        }
    }

}
