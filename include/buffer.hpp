#ifndef BUFFER_H
#define BUFFER_H

#include <cstdio>

class Buffer {
   public:
    const int width;
    const int height;
    const int n_channel;

    Buffer(const int width, const int height, const int n_channel = 1,
           const float init_val = 0.f);
    ~Buffer();

    inline float &at(const int x, const int y, const int channel = 0) const {
        return data[(y * width + x) * n_channel + channel];
    }

   private:
    float *data;
};

#endif