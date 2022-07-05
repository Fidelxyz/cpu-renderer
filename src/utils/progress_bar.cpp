#include "utils/progress_bar.hpp"

#include <cstdio>

ProgressBar::ProgressBar(const std::string &name, const int total_count) {
    this->name = name;
    this->total_count = total_count;
    display();
}

void ProgressBar::update() {
    if (current_count >= total_count) {
        return;
    }
    current_count++;
    display();
}

void ProgressBar::display() {
    printf("\r%s: [", name.c_str());
    int progress_length = BAR_LENGTH * (static_cast<float>(current_count) /
                                        static_cast<float>(total_count));
    for (int i = 0; i < progress_length; i++) {
        putchar('=');
    }
    for (int i = BAR_LENGTH - progress_length; i; i--) {
        putchar('-');
    }
    printf("] %d / %d", current_count, total_count);
    if (current_count == total_count) {
        putchar('\n');
    }
    fflush(stdout);
}