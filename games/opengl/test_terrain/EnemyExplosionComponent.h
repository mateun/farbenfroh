//
// Created by mgrus on 16.03.2025.
//

#ifndef ENEMYEXPLOSIONCOMPONENT_H
#define ENEMYEXPLOSIONCOMPONENT_H



/**
* This is spawned/activated when an enemy shall explode.
*/
class EnemyExplosionComponent {

public:
   EnemyExplosionComponent(Mesh* debrisMesh, Texture* texture, glm::vec3 location);
   void update();
   void render(Camera* camera);

private:
   gru::ParticleEmitter* smokeEmitter;
   glm::vec3 location;
   Texture* texture = nullptr;
   Mesh * mesh = nullptr;
   bool active = true;
   float lifetime = 0;
};



#endif //ENEMYEXPLOSIONCOMPONENT_H
