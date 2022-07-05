#pragma once
#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include <string>

class ProgressBar {
   private:
    static const int BAR_LENGTH = 30;

    std::string name;
    int total_count;
    int current_count = 0;

   public:
    ProgressBar(const std::string &name, const int total_count);
    void update();

   private:
    void display();
};

#endif