//
// Created by mgrus on 19.03.2025.
//

#ifndef PLAYERBULLETCOMP_H
#define PLAYERBULLETCOMP_H



class PlayerBulletComp : public ScriptComponent{
public:
    PlayerBulletComp(const std::vector<std::shared_ptr<SceneNode>>& enemyNodes);
    void update() override;

private:
    float lifeTime = 0.0f;
    float maxLifeInSeconds = 2;
    const std::vector<std::shared_ptr<SceneNode>>& _enemyNodes;
};



#endif //PLAYERBULLETCOMP_H
