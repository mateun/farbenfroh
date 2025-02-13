//
// Created by mgrus on 19.10.2024.
//

#ifndef SIMPLE_KING_KING_GAME_H
#define SIMPLE_KING_KING_GAME_H
#include "engine/game/default_app.h"

namespace king {

    class KingGame : public DefaultGame {
    public:
        void update() override;
        void init() override;
        void render() override;
        bool shouldStillRun() override;
        std::string getAssetFolder() override;
        std::string getName() override;

    };

} // king

#endif //SIMPLE_KING_KING_GAME_H
