//
// Created by mgrus on 06.12.2024.
//

#include "SoftwareRenderedGame.h"

void SoftwareRenderedGame::update() {
}

void SoftwareRenderedGame::init() {
    DefaultGame::init();

    backbufferTexture = createEmptyTexture(320, 200);
    auto bm = backbufferTexture->bitmap->pixels;
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 200; y++) {
            int pixelOffset = (x + y *320 ) * 4;
            bm[pixelOffset + 0] = 155;
            bm[pixelOffset+1] = 150;
            bm[pixelOffset+2] = 5;
            bm[pixelOffset +3] = 255;
        }
    }
    updateTextTexture(backbufferTexture->bitmap->width, backbufferTexture->bitmap->height, backbufferTexture);

}

void SoftwareRenderedGame::render() {
    bindCamera(getUICamera());
    lightingOff();

    scale({320, 200,1});
    location(glm::vec3{400, 300, -1});
    bindTexture(backbufferTexture);
    drawPlane();


}

bool SoftwareRenderedGame::shouldStillRun() {
    return true;
}

std::vector<std::string> SoftwareRenderedGame::getAssetFolder() {
    return {"../games/software_rendered_king/assets"};
}

bool SoftwareRenderedGame::shouldAutoImportAssets() {
    return true;
}
