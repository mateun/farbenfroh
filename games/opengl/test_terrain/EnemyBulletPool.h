//
// Created by mgrus on 21.03.2025.
//

#ifndef ENEMYBULLETPOOL_H
#define ENEMYBULLETPOOL_H

// #include <vector>


class SceneNode;

/**
* This class manages every enemy bullet
* and the recycling of the enemy bullets.
* TODO handle the different patterns?
*/
class EnemyBulletPool {

public:
    EnemyBulletPool();

private:
  std::vector<std::shared_ptr<SceneNode>> _bullets;

};



#endif //ENEMYBULLETPOOL_H
