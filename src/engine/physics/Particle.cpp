//
// Created by mgrus on 15.09.2024.
//

#include "Particle.h"

cyclone::Particle::Particle(glm::vec3 initialPosition, glm::vec3 initialVelocity, float initialMass)  {
    this->position = initialPosition;
    this->velocity = initialVelocity;
    if (initialMass == 0) {
        this->inverseMass = MAXINT32;
    } else {
        this->inverseMass = 1 / initialMass;
    }
    this->damping = 0.99;

    clearAccumulator();

}

void cyclone::Particle::integrate(float duration) {
    if (inverseMass <= 0.0f) return;

    if (duration <= 0) return;

    // Update linear position
    position += (velocity * duration);

    // Gravity is always present:
    acceleration = glm::vec3(0, -9.81, 0);

    // Work out acceleration from the force.
    glm::vec3 resultingAcc = acceleration;
    resultingAcc += (forceAccum * inverseMass);

    // Update linear velocity.
    velocity += (resultingAcc * duration);

    // Impose drag.
    float frameDamping = powf(damping, duration);
    velocity *= frameDamping;

    clearAccumulator();

}

void cyclone::Particle::addForce(glm::vec3 force) {
    forceAccum += force;
}

glm::vec3 cyclone::Particle::getPosition() {
    return position;
}

void cyclone::Particle::clearAccumulator() {
    forceAccum = {0, 0, 0};
}