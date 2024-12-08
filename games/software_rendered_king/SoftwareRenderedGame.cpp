//
// Created by mgrus on 06.12.2024.
//

#include "SoftwareRenderedGame.h"

void SoftwareRenderedGame::update() {
}

void SoftwareRenderedGame::clearBackBuffer() {
#ifdef PROFILE_ON
    LARGE_INTEGER start;
    QueryPerformanceCounter(&start);
#endif

    void* bm = backbufferTexture->bitmap->pixels;
    int size_in_bytes = backbufferTexture->bitmap->width * backbufferTexture->bitmap->height * 4;

    // Number of 256-bit AVX registers needed (256 bits = 32 bytes)
    size_t num_avx_registers = size_in_bytes  / 32;

    // Pointer to the 256-bit (32-byte) buffer
    __m256i* buf256bit = static_cast<__m256i*>(bm);

    // Create the mask where every 4th byte is 255 (0xFF)
    alignas(32) const uint8_t pattern[32] = {
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    };
    __m256i mask = _mm256_load_si256(reinterpret_cast<const __m256i*>(pattern));

    // Loop through the buffer in 32-byte chunks
    for (size_t i = 0; i < num_avx_registers; ++i) {
        _mm256_store_si256(&buf256bit[i], mask);
    }

    // Handle any remaining bytes (if size is not a multiple of 32)
    size_t remaining_bytes = size_in_bytes % 32;
    if (remaining_bytes > 0) {
        uint8_t* tail = reinterpret_cast<uint8_t*>(buf256bit + num_avx_registers);
        for (size_t i = 0; i < remaining_bytes; ++i) {
            tail[i] = 0;
        }
    }

#ifdef PROFILE_ON
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);
    auto diff = end.QuadPart - start.QuadPart;
    printf("Ticks elapsed: %lld time(s): %f  (micros): %f \n", diff,  (float) diff / (float) performanceFrequency, (float) diff / (float) performanceFrequency * 1000 * 1000);
#endif

    //ZeroMemory(bm, backbufferTexture->bitmap->width * backbufferTexture->bitmap->height * 4);
}

void SoftwareRenderedGame::drawPixel(int x, int y, glm::vec4 color) {
    int pixelOffset = (x + y *backbufferTexture->bitmap->width ) * 4;

    // Do not draw if pixel is out of bounds:
    if (pixelOffset >= backbufferTexture->bitmap->width * backbufferTexture->bitmap->height * 4) {
        return;
    }

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
            dy = -dy;
            glm::vec2 buffer = to;
            to = from;
            from = buffer;
        }

        float y = 0;
        float step = dy / dx;
        for (int x = 0; x < dx; x++) {
            drawPixel(from.x + x, from.y + y, color);
            y += step;
        }
    } else {
        // y dominant
        if (dy < 0) {
            dy = -dy;
            dx = -dx;
            glm::vec2 buffer = to;
            to = from;
            from = buffer;
        }

        float x = 0;
        float step = dx / dy;
        for (int y = 0; y < dy; y++) {
            drawPixel(from.x + x, from.y + y, color);
            x += step;
        }
    }

}

void SoftwareRenderedGame::drawTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec4 col) {

    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    drawLine(a, b, col);
    drawLine(a, c, col);
    drawLine(b, c, col);

}

void SoftwareRenderedGame::init() {
    DefaultGame::init();
    glDefaultObjects->currentRenderState->clearColor = glm::vec4(0, 0.1, 0.1, 1);

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

glm::vec3 SoftwareRenderedGame::transformObjectToScreenSpace(glm::vec3 objectPosition, glm::mat4 matworld) {
    auto clip =  currentProjectionMatrix * currentViewMatrix * matworld * glm::vec4(objectPosition, 1.0);
    auto ndc = glm::vec3(clip.x / clip.w, clip.y / clip.w, clip.z / clip.w);
    auto ss = glm::vec3 {((ndc.x + 1)/2) * backbufferTexture->bitmap->width, ((ndc.y+1)/2)* backbufferTexture->bitmap->height, ((ndc.z+1)/2)};
    return ss;
}

void SoftwareRenderedGame::render() {

    // Software rendering logic.
    // Here we write directly to our own backbuffer which
    // is represented as an OpenGL texture.
    clearBackBuffer();

    // Draw some lines directly in 2d coordinates
    // drawLine({10, 10}, {100, 12}, {200, 10, 10, 25});
    // drawLine({100, 25}, {10, 20}, {120, 10, 10, 25});
    // drawLine({10, 30}, {100, 120}, {100, 20, 10, 25});
    // drawLine({100, 140}, {10, 40}, {100, 20, 10, 25});
    // drawLine({100, 199}, {10, 42}, {100, 20, 10, 25});
    // drawLine({105, 222}, {0, 43}, {80, 50, 10, 25});
    // drawLine({0, 0}, {320, 200}, {80, 50, 60, 25});
    // drawLine({320, 0}, {0, 200}, {80, 50, 60, 25});
    // drawLine({0, 0}, {0, 200}, {80, 50, 60, 25});
    // drawLine({319, 0}, {319, 200}, {80, 50, 60, 25});
    // drawLine({0, 199}, {319, 199}, {80, 50, 60, 25});
    // drawLine({0, 0}, {319, 0}, {80, 50, 60, 25});

    // Now some 3d drawing:
    // Do the whole pipeline here for simplicity sake:
    std::vector<glm::vec3> pos = {
        {-0.5, -0.5, -2},
        {0.5, -0.5, -2},
        {0, 0.5, -2},
    };
    std::vector<glm::vec3> screenSpace = {};

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 3.0f, 5.0f), // Camera position
                                glm::vec3(0.0f, 0.0f, 0.0f), // Look-at point
                                glm::vec3(0.0f, 1.0f, 0.0f)); // Up direction
    glm::mat4 proj = glm::perspectiveFov<float>(glm::radians(60.0f), 320, 200, 0.1, 400.0);

    // for (auto p : pos) {
    //     auto clip =  proj * view* glm::vec4(p, 1.0);
    //     auto ndc = glm::vec3(clip.x / clip.w, clip.y / clip.w, clip.z / clip.w);
    //     auto ss = glm::vec3 {((ndc.x + 1)/2) * backbufferTexture->bitmap->width, ((ndc.y+1)/2)* backbufferTexture->bitmap->height, ((ndc.z+1)/2)};
    //     screenSpace.push_back(ss);
    // }


    currentProjectionMatrix = proj;
    currentViewMatrix = view;

    // A grid on the ground
    glm::mat4 matworld = glm::mat4(1.0f);

    for (int i = 0; i < 20; i++) {
        auto s0 = transformObjectToScreenSpace({-10, 0, 5 - i}, matworld);
        auto s1 = transformObjectToScreenSpace({10, 0, 5 - i}, matworld);
        drawLine(s0, s1, {20, 20, 20, 255});
    }

    for (int i = 0; i < 10; i++) {
        auto s0 = transformObjectToScreenSpace({-5+i, 0, 4}, matworld);
        auto s1 = transformObjectToScreenSpace({-5+i, 0, -155}, matworld);
        drawLine(s0, s1, {20, 20, 20, 255});
    }




    // The triangle (player)
    for (auto p : pos) {
        glm::mat4 matworld = glm::mat4(1.0f);
        auto ssPos = transformObjectToScreenSpace(p, matworld);
        screenSpace.push_back(ssPos);
    }
    drawTriangle(screenSpace[0], screenSpace[1], screenSpace[2], glm::vec4(255, 200, 20, 255));

    updateTextTexture(backbufferTexture->bitmap->width, backbufferTexture->bitmap->height, backbufferTexture);

    // Draw the backbuffer texture to the screen:
    bindCamera(getUICamera());
    lightingOff();
    scale({backbufferTexture->bitmap->width*2, backbufferTexture->bitmap->height*2, 1});
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
