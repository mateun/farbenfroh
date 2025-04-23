//
// Created by mgrus on 02.02.2025.
//

#ifndef SHMUP_H
#define SHMUP_H

#include "../../../src/engine/game/default_game.h"



class Shmup : public DefaultGame {

    void update() override;
    void init() override;
    void render() override;
    bool shouldStillRun() override;
    std::vector<std::string> getAssetFolder() override;
    bool shouldAutoImportAssets() override;

};



#endif //SHMUP_H
