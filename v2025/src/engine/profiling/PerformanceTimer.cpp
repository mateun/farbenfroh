//
// Created by mgrus on 15.03.2025.
//

#include "PerformanceTimer.h"


PerformanceTimer::PerformanceTimer(bool startImmediately) {
    if (!frequencyInitialized) {
        frequencyInitialized = true;
        QueryPerformanceFrequency(&frequency);
    }
    if (startImmediately) start();
}

void PerformanceTimer::start() {
    QueryPerformanceCounter(&startTime);
}

void PerformanceTimer::stop() {
    QueryPerformanceCounter(&endTime);
    diffInSeconds = (float)(endTime.QuadPart - startTime.QuadPart) / (float) frequency.QuadPart;


}

float PerformanceTimer::durationInSeconds() const {
    return diffInSeconds;
}
