//
// Created by mgrus on 20.10.2024.
//

#ifndef SIMPLE_KING_INSIDESHIPSTATE_H
#define SIMPLE_KING_INSIDESHIPSTATE_H

#include "king_game.h"
#include "physics.h"

namespace king {

/**
 * This class handles the game while being on the ship.
 *
 */
class InsideShipState : public KingGameState {

    public:
        InsideShipState(KingGame *game, JPH::BodyInterface &bodyInterface);

        void init() override;

        void update() override;

        void render() override;

    private:
        JPH::BodyInterface &bodyInterface;
        JPH::BodyID sphereId;
        fireball::Sphere *ball;
    };

}

#endif //SIMPLE_KING_INSIDESHIPSTATE_H
