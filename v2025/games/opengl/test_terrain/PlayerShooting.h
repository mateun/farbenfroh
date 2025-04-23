//
// Created by mgrus on 21.03.2025.
//

#ifndef PLAYERSHOOTINGCOMP_H
#define PLAYERSHOOTINGCOMP_H

#include <engine/game/NodeComponent.h>


class PlayerShooting {
public:
  PlayerShooting(const std::shared_ptr<SceneNode>& playerNodem, const std::shared_ptr<PlayerBulletPool>& bulletPool);
  void update();

private:
    std::vector<std::shared_ptr<SceneNode>> playerBulletPool;
    std::shared_ptr<SceneNode> _playerNode;
    std::shared_ptr<PlayerBulletPool> _bulletPool;
};



#endif //PLAYERSHOOTINGCOMP_H
