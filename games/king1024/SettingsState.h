//
// Created by mgrus on 03.11.2024.
//

#ifndef SIMPLE_KING_SETTINGSSTATE_H
#define SIMPLE_KING_SETTINGSSTATE_H
#include "king_game.h"
#include <map>
#include <vector>

namespace  king {

    class SettingsState : public KingGameState {

    public:
        SettingsState(KingGame* game);

        void update() override;
        void render() override;
        void init() override;

    private:
        std::vector<std::string> activeMonitorDeviceNames;
        std::map<std::string, std::vector<MonitorResolution>> resolutions;
        int resIndex = 0;
    };

}




#endif //SIMPLE_KING_SETTINGSSTATE_H
