//
// Created by mgrus on 15.09.2024.
//

#ifndef SIMPLE_KING_PARTICLEFORCEGENERATOR_H
#define SIMPLE_KING_PARTICLEFORCEGENERATOR_H

class Particle;

class ParticleForceGenerator {

public:
    virtual void updateForce(Particle* particle, float duration) = 0;

};


#endif //SIMPLE_KING_PARTICLEFORCEGENERATOR_H
