//
// Created by mgrus on 15.03.2025.
//

#include "PerformanceTimer.h"

extern int64_t perframance_frequency;

PerformanceTimer::PerformanceTimer(bool startImmediately) {
    if (startImmediately) start();
}

void PerformanceTimer::start() {
    QueryPerformanceCounter(&startTime);
}

void PerformanceTimer::stop() {
    QueryPerformanceCounter(&endTime);
    diffInSeconds = (float)(endTime.QuadPart - startTime.QuadPart) / (float) performance_frequency;
}

float PerformanceTimer::durationInSeconds() const {
    return diffInSeconds;
}
