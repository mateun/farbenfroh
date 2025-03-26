//
// Created by mgrus on 25.03.2025.
//

#include "FpsCounter.h"

#include <vector>
#include <engine/game/Timing.h>

float FpsCounter::frameTimeInMicroSecondsAvg() {
  return ftMicrosAvg;
}

float FpsCounter::lastFPSAverage() {
  return lastAvgFPS;
}

void FpsCounter::update() {
    float ftSeconds = Timing::lastFrameTimeInSeconds();
    static std::vector<float> ftbuffer(1000);
    static std::vector<float> ftMicrosbuffer(1000);

    float ftMicros = (float) ftSeconds * 1000.0f * 1000.0f;
    currentFrame++;

    // Calculate average frametimes and fps.
    // Every 1000 frames we take the average.
    // Clear the buffer and collect the next 1000 frames for averaging.
    ftbuffer.push_back(ftSeconds);
    ftMicrosbuffer.push_back(ftMicros);
    if (ftbuffer.size() > 999) {
        float sum = 0;
        float sumMicros = 0;
        for (int i = 0; i < ftbuffer.size(); i++) {
            sum += ftbuffer[i];
            sumMicros += ftMicrosbuffer[i];
        }
        ftMicrosAvg = sumMicros / ftbuffer.size();
        ftSecondsAvg = sum / ftbuffer.size();
        lastAvgFPS = 1/ftSecondsAvg;
        ftbuffer.clear();
        ftMicrosbuffer.clear();
    }
 }
