//
// Created by mgrus on 20.03.2025.
//

#include "EnemyHitManager.h"

void EnemyHitManager::reportBulletHit(PlayerBulletComp *player_bullet_comp) {
    // TODO deal damage to this enemy node.
    // for now just disable the node.
    getNode()->disable();
}
