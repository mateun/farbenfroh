//
// Created by mgrus on 06.12.2024.
//

#include "SoftwareRenderedGame.h"

void SoftwareRenderedGame::update() {
}

void SoftwareRenderedGame::clearBackBuffer() {
    auto bm = backbufferTexture->bitmap->pixels;
    ZeroMemory(bm, backbufferTexture->bitmap->width * backbufferTexture->bitmap->height * 4);
}

void SoftwareRenderedGame::drawPixel(int x, int y, glm::vec4 color) {
    int pixelOffset = (x + y *backbufferTexture->bitmap->width ) * 4;
    auto bm = backbufferTexture->bitmap->pixels;
    bm[pixelOffset + 0] = color.r;
    bm[pixelOffset+1] = color.g;
    bm[pixelOffset+2] = color.b;
    bm[pixelOffset +3] = color.a;
}

void SoftwareRenderedGame::drawLine(glm::vec2 from, glm::vec2 to, glm::vec4 color) {
    auto dx = to.x - from.x;
    auto dy = to.y - from.y;


    // if (dy < 0) {
    //     dy = -dy;
    //     glm::vec2 buffer = to;
    //     to = from;
    //     from = buffer;
    // }

    // Calculate dominance, x or y.
    // For the dominant side, we loop in whole steps,
    // for the non dominant side we loop in the fractured steps.
    // We always go left right, bottom to top.
    // If the incoming line is not conforming, re-frame it so it does.
    if (abs(dx) > abs(dy)) {
        // x dominant

        if (dx < 0) {
            dx = -dx;
            glm::vec2 buffer = to;
            to = from;
            from = buffer;
        }

        float y = 0;
        float step = abs(dy) / abs(dx);
        for (int x = 0; x < dx; x++) {
            drawPixel(from.x + x, from.y + y, color);
            y += step;
        }
    } else {
        // y dominant

        if (dy < 0) {
            dy = -dy;
            glm::vec2 buffer = to;
            to = from;
            from = buffer;
        }

        float x = 0;
        float step = abs(dx) / abs(dy);
        for (int y = 0; y < dy; y++) {
            drawPixel(from.x + x, from.y + y, color);
            x += step;
        }
    }



}

void SoftwareRenderedGame::init() {
    DefaultGame::init();

    backbufferTexture = createEmptyTexture(320, 200);


    // Clear memory performance test
    // LARGE_INTEGER start;
    // QueryPerformanceCounter(&start);
    //ZeroMemory(bm, 320 * 200 * 4);
    // LARGE_INTEGER end;
    // QueryPerformanceCounter(&end);
    // auto diff = end.QuadPart - start.QuadPart;
    // printf("Ticks elapsed: %lld time(s): %f  (micros): %f \n", diff,  (float) diff / (float) performanceFrequency, (float) diff / (float) performanceFrequency * 1000 * 1000);


    // Clear memory with loop:
    // This is 4 times slower:
    // QueryPerformanceCounter(&start);
    // uint64_t* buf64bit = (uint64_t*)bm;
    // for (int x = 0; x < (1920 * 1080 / 2); x++) {
    //     buf64bit[x] = 0;
    // }
    // QueryPerformanceCounter(&end);
    // diff = end.QuadPart - start.QuadPart;
    // printf("Ticks elapsed: %lld time(s): %f  (micros): %f \n", diff,  (float) diff / (float) performanceFrequency, (float) diff / (float) performanceFrequency * 1000 * 1000);




}

void SoftwareRenderedGame::render() {
    bindCamera(getUICamera());
    lightingOff();

    clearBackBuffer();
    auto bm = backbufferTexture->bitmap->pixels;
    for (int x = 0; x < backbufferTexture->bitmap->width; x++) {
        int y = 100;
        drawPixel(x,y, glm::vec4(160, 120, 0, 255));
    }

    drawLine({10, 10}, {100, 12}, {200, 10, 10, 255});
    drawLine({100, 25}, {10, 20}, {120, 10, 10, 255});
    drawLine({10, 30}, {100, 120}, {100, 20, 10, 255});
    drawLine({100, 140}, {10, 40}, {100, 20, 10, 255});



    updateTextTexture(backbufferTexture->bitmap->width, backbufferTexture->bitmap->height, backbufferTexture);

    // Draw the texture to the screen
    scale({backbufferTexture->bitmap->width, backbufferTexture->bitmap->height, 1});
    location(glm::vec3{400, 300, -1});
    bindTexture(backbufferTexture);
    drawPlane();
    renderFPS();

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
