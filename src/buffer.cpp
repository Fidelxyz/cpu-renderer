#include "buffer.hpp"

#include <algorithm>

Buffer::Buffer(const int width, const int height, const int n_channel,
               const float init_val)
    : width(width),
      height(height),
      n_channel(n_channel),
      data(new float[width * height * n_channel]) {
    std::fill(data, data + width * height * n_channel, init_val);
}

Buffer::~Buffer() { delete[] data; }