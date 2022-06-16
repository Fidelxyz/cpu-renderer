#pragma once
#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <string>

class Timer {
   public:
    Timer(const std::string &name);
    ~Timer();
    void stop();

   private:
    const std::string name;
    const std::chrono::high_resolution_clock::time_point start_time;
    bool is_stopped = false;
};

#endif