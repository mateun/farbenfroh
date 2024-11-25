//
// Created by mgrus on 03.11.2024.
//

#include "SettingsState.h"
#include "graphics.h"

void king::SettingsState::update() {
    auto numResolutions = resolutions[activeMonitorDeviceNames[0]].size();
    if (controllerButtonPressed(ControllerButtons::DPAD_DOWN, 0)) {
        if (resIndex < numResolutions) {
            resIndex++;
        }
    }

    if (controllerButtonPressed(ControllerButtons::DPAD_UP, 0)) {
        if (resIndex > 0) {
            resIndex--;
        }
    }

    if (controllerButtonPressed(ControllerButtons::X_BUTTON, 0)) {
        auto res = resolutions[activeMonitorDeviceNames[0]][resIndex];
        if (changeResolution(res.width, res.height, res.frequency, activeMonitorDeviceNames[0], true)) {
            printf("ok!\n");
            glViewport(0, 0, scaled_width, scaled_height);
        } else {
            exit(9999);
        }
    }

    if (controllerButtonPressed(ControllerButtons::B_BUTTON, 0)) {
        kingGame->switchState(EGameState::InMainMenu, true, false);
    }

}

void king::SettingsState::render() {

    bindCamera(kingGame->getUICamera());
    lightingOff();
    flipUvs(false);
    foregroundColor({.1, .7, .1, 1});

    auto scaleFactor = glDefaultObjects->currentRenderState->textScale;
    int y = scaled_height - (150/dpiScaleFactor);
    int x = 256 * scaleFactor.x;

    auto device = activeMonitorDeviceNames[0];
    kingGame->getButtonFont()->renderText(device, {x, y, -1.5});
    kingGame->getButtonFont()->renderText(std::to_string(resIndex) + "/" + std::to_string(resolutions[device].size()), {x+50, y - scaleFactor.y*35, -1.5});
    auto res = resolutions[device][resIndex];
    std::string text = "Res: " + std::to_string(res.width) + "x" + std::to_string(res.height) + " (" + std::to_string(res.frequency) + "hz)";
    kingGame->getButtonFont()->renderText(text, {x+50, y - 2*scaleFactor.y* 35, -1.5});

}

void king::SettingsState::init() {
    activeMonitorDeviceNames = getAllActiveMonitors();
    for (auto device : activeMonitorDeviceNames) {
        resolutions[device] = getMonitorResolutions(device);
    }

    glDefaultObjects->currentRenderState->clearColor = {0, 0, 0, 1};
    glDefaultObjects->currentRenderState->textScale = {2, 2};

}

king::SettingsState::SettingsState(king::KingGame *game) : KingGameState(game) {

}
