#ifndef BUFFER_H
#define BUFFER_H

#include <algorithm>
#include <cstdio>

template <typename T>
class Buffer {
   public:
    const int width;
    const int height;
    const int n_channels;
    T *data;

    Buffer(const int width, const int height, const int n_channels = 1);
    Buffer(const int width, const int height, const int n_channels,
           const T init_val);
    ~Buffer();

    inline T &at(const int x, const int y, const int channel = 0) const {
        return data[(y * width + x) * n_channels + channel];
    }
};

template <typename T>
Buffer<T>::Buffer(const int width, const int height, const int n_channels,
                  const T init_val)
    : width(width),
      height(height),
      n_channels(n_channels),
      data(new T[width * height * n_channels]) {
    std::fill(data, data + width * height * n_channels, init_val);
}

template <typename T>
Buffer<T>::Buffer(const int width, const int height, const int n_channels)
    : width(width),
      height(height),
      n_channels(n_channels),
      data(new T[width * height * n_channels]) {}

template <typename T>
Buffer<T>::~Buffer() {
    delete[] data;
}

#endif