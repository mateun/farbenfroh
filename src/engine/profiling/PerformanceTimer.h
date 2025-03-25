//
// Created by mgrus on 15.03.2025.
//

#ifndef PERFORMANCETIMER_H
#define PERFORMANCETIMER_H
#include <Windows.h>

class PerformanceTimer {

public:
    explicit PerformanceTimer(bool startImmediately = false);
    void start();
    void stop();
    float durationInSeconds() const;

private:
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    LARGE_INTEGER frequency;
    float diffInSeconds = 0.0f;
    inline static bool frequencyInitialized = false;

};



#endif //PERFORMANCETIMER_H
