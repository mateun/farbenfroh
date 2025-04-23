//
// Created by mgrus on 20.03.2025.
//

#ifndef ENEMYHITMANAGER_H
#define ENEMYHITMANAGER_H

#include <engine/game/NodeComponent.h>

class EnemyHitManager : public NodeComponent {
public:
    void reportBulletHit(PlayerBulletComp * player_bullet_comp);
};



#endif //ENEMYHITMANAGER_H
