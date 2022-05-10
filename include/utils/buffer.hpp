#ifndef BUFFER_H
#define BUFFER_H

#include <algorithm>
#include <cstdio>

template <typename T>
class Buffer {
   public:
    const size_t width;
    const size_t height;
    const size_t n_channels;
    T *data;

    Buffer(const size_t width, const size_t height,
           const size_t n_channels = 1);
    Buffer(const size_t width, const size_t height, const size_t n_channels,
           const T init_val);
    ~Buffer();

    inline T &at(const size_t x, const size_t y,
                 const size_t channel = 0) const {
        return data[(y * width + x) * n_channels + channel];
    }
};

template <typename T>
Buffer<T>::Buffer(const size_t width, const size_t height,
                  const size_t n_channels, const T init_val)
    : width(width),
      height(height),
      n_channels(n_channels),
      data(new T[width * height * n_channels]) {
    std::fill(data, data + width * height * n_channels, init_val);
}

template <typename T>
Buffer<T>::Buffer(const size_t width, const size_t height,
                  const size_t n_channels)
    : width(width),
      height(height),
      n_channels(n_channels),
      data(new T[width * height * n_channels]) {}

template <typename T>
Buffer<T>::~Buffer() {
    delete[] data;
}

#endif