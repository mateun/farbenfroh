//
// Created by mgrus on 03.02.2025.
//

#ifndef SERVERVIZ_H
#define SERVERVIZ_H

#include "../../../src/engine/game/default_game.h"

class ServerViz : public DefaultGame {

    void update() override;
    void render() override;
    bool shouldStillRun() override;
    bool shouldAutoImportAssets() override;
    std::vector<std::string> getAssetFolder() override;

};



#endif //SERVERVIZ_H
