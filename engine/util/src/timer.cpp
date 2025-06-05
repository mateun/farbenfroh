//
// Created by mgrus on 28.05.2025.
//

// #include "../util/include/util.h"
#include <util.h>

// Private variables needed for high performance timing on windows:
static LARGE_INTEGER freq;
static bool freq_initialized = false;

// TODO make multi-platform
void start_timer(Timer &timer) {
    if (!freq_initialized) {
        QueryPerformanceFrequency(&freq);
    }
    QueryPerformanceCounter(&timer.start_ticks);
}

void end_timer(Timer &timer) {
    QueryPerformanceCounter(&timer.end_ticks);
    timer.diff_ticks = timer.end_ticks.QuadPart - timer.start_ticks.QuadPart;
}

float timer_seconds(Timer& timer) {
    return timer.diff_ticks / (float) freq.QuadPart;

}

float timer_microseconds(Timer& timer) {
    auto seconds = timer_seconds(timer);
    return seconds * 1000.0f * 1000.0f;
}