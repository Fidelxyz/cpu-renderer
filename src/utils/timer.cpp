#include "utils/timer.hpp"

#include <cstdio>

Timer::Timer(const std::string &name)
    : name(name), start_time(std::chrono::high_resolution_clock::now()) {}

Timer::~Timer() {
    if (!is_stopped) stop();
}

void Timer::stop() {
    if (is_stopped) return;
    is_stopped = true;
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    printf("[Timer] %s: %ldus\n", name.c_str(), duration.count());
}