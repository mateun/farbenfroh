//
// Created by mgrus on 21.03.2025.
//

#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H


class EnemyBulletPool;
/**
* This class maanges all kinds of collisions:
* - playerBullets vs. Enemies
* - enemyBullets vs. Players
* - playerBullets vs. Walls (Obstacles in general)
*/
class CollisionManager {
    public:
    // The important ones
    void handlePlayerBulletsVsEnemies(PlayerBulletPool* playerBulletPool, const std::vector<std::shared_ptr<SceneNode>>& enemies);
    void handleEnemyBulletsVsPlayer(SceneNode* playerNode, EnemyBulletPool* enemyBulletPool);

    // These are maybe not even necessary:
    void handlePlayerBulletsVsEnvironment();
    void handleEnemyBulletsVsEnvironment();

};



#endif //COLLISIONMANAGER_H
