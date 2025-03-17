//
// Created by mgrus on 16.03.2025.
//

#include "EnemyExplosionComponent.h"

EnemyExplosionComponent::EnemyExplosionComponent(Mesh* debrisMesh, Texture* texture, glm::vec3 location) : location(location), mesh(debrisMesh), texture(texture) {
    smokeEmitter = new gru::ParticleEmitter(nullptr,  texture, gru::EmitterType::EXPLOSION, location, 50);
    // TODO acd actual debris/explosion emitter
    //explosionEmitter = new gru::ParticleSystem(debrisMesh, texture, location, 50);
}

void EnemyExplosionComponent::update() {
    if (!active) {
        return;
    }
    lifetime += ftSeconds;
    if (lifetime > 1.1) {
        smokeEmitter->disable();
        active = false;
    } else {
        smokeEmitter->update();
    }

}

void EnemyExplosionComponent::render(Camera* camera) {
    smokeEmitter->draw(camera);
}
