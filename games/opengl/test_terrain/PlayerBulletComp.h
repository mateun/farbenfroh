//
// Created by mgrus on 19.03.2025.
//

#ifndef PLAYERBULLETCOMP_H
#define PLAYERBULLETCOMP_H



class PlayerBulletComp : public ScriptComponent{
public:
    PlayerBulletComp();
    void update() override;

private:
    float lifeTime = 0.0f;
    float maxLifeInSeconds = 2;

};



#endif //PLAYERBULLETCOMP_H
