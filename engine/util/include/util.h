//
// Created by mgrus on 28.05.2025.
//

#ifndef UTIL_H
#define UTIL_H
#include <engine.h>

struct Timer {
    LARGE_INTEGER start_ticks;
    LARGE_INTEGER end_ticks;
    LONGLONG diff_ticks;
};

void start_timer(Timer& timer);
void end_timer(Timer& timer);
float timer_seconds(Timer& timer);
float timer_microseconds(Timer& timer);

std::vector<uint8_t> read_file_binary_to_vector(const std::string& filename);
uint8_t* read_file_binary(const std::string& fileName);


#endif //UTIL_H
