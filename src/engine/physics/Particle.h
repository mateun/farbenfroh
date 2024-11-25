//
// Created by mgrus on 15.09.2024.
//

#ifndef SIMPLE_KING_PARTICLE_H
#define SIMPLE_KING_PARTICLE_H

#include <glm/glm.hpp>
namespace cyclone {

    class Particle {

    public:
        Particle(glm::vec3 initialPosition, glm::vec3 initialVelocity, float initialMass);

    public:
        // The main workhorse of this engine.
        // Derives the position from all forces acting on this particle.
        void integrate(float duration);

        void addForce(glm::vec3 force);

        glm::vec3 getPosition();

    private:
        // Is called after each integration step
        void clearAccumulator();

    protected:
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        glm::vec3 forceAccum;

        float damping;
        float inverseMass;

    };


}

#endif //SIMPLE_KING_PARTICLE_H
