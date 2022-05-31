#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <Eigen/Core>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <string>

#include "global.hpp"

template <typename T>
class Texture {
   public:
    size_t width;
    size_t height;

   private:
    T *data;

   public:
    Texture();
    Texture(const size_t width, const size_t height);
    Texture(const size_t width, const size_t height, const T init_val);
    Texture(Texture &&src);
    ~Texture();

    Texture<T> &operator=(Texture<T> &&src);

    inline T &at(const size_t x, const size_t y) const;
    inline T &at(const size_t index) const;
    inline T &operator[](const size_t index) const;

    inline bool is_null() const;

    void read_img(const std::string &filename, const bool linear);
    void write_img(const std::string &filename, const bool linear) const;

    static float gamma_correction(const float val, const float gamma);
    static vec3 gamma_correction(const vec3 &val, const float gamma);

    static float truncate_color(float color);
    static vec3 truncate_color(const vec3 &color);

    T sample(const vec2 &uv) const;
};

template <typename T>
Texture<T>::Texture() {
    width = 0;
    height = 0;
    data = nullptr;
}

template <typename T>
Texture<T>::Texture(const size_t width, const size_t height) {
    this->width = width;
    this->height = height;
    this->data = new T[width * height];
}

template <typename T>
Texture<T>::Texture(const size_t width, const size_t height, const T init_val)
    : Texture(width, height) {
    std::fill(data, data + width * height, init_val);
}

template <typename T>
Texture<T>::Texture(Texture &&src) {
    width = std::move(src.width);
    height = std::move(src.height);
    data = std::move(src.data);
    src.data = nullptr;
}

template <typename T>
Texture<T>::~Texture() {
    delete[] data;
}

template <typename T>
Texture<T> &Texture<T>::operator=(Texture &&src) {
    width = std::move(src.width);
    height = std::move(src.height);
    data = std::move(src.data);
    src.data = nullptr;
    return *this;
}

template <typename T>
inline T &Texture<T>::at(const size_t x, const size_t y) const {
    return data[y * width + x];
}

template <typename T>
inline T &Texture<T>::at(const size_t index) const {
    return data[index];
}

template <typename T>
inline T &Texture<T>::operator[](const size_t index) const {
    return data[index];
}

template <typename T>
inline bool Texture<T>::is_null() const {
    return data == nullptr;
}

template <typename T>
void Texture<T>::read_img(const std::string &filename, const bool linear) {
    // only support Gray and RGB image
    static_assert(std::is_same<T, float>::value ||
                  std::is_same<T, vec3>::value);

    cv::Mat img;
    if constexpr (std::is_same<T, float>::value) {  // float
        img = cv::imread(filename, cv::IMREAD_GRAYSCALE);
    } else {  // vec3
        img = cv::imread(filename, cv::IMREAD_COLOR);
    }
    // re-allowcate space
    if (static_cast<int>(width) != img.rows ||
        static_cast<int>(height) != img.cols) {
        width = img.rows;
        height = img.cols;
        delete[] data;
        data = new T[width * height];
    }

    // write data
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            if constexpr (std::is_same<T, float>::value) {  // float
                at(x, y) = img.at<uchar>(y, x) / 255.f;
            } else {  // vec3
                at(x, y)[0] = img.at<cv::Vec3b>(y, x)[2] / 255.f;
                at(x, y)[1] = img.at<cv::Vec3b>(y, x)[1] / 255.f;
                at(x, y)[2] = img.at<cv::Vec3b>(y, x)[0] / 255.f;
            }
            if (!linear) at(x, y) = gamma_correction(at(x, y), 2.2f);
        }
    }
}

template <typename T>
void Texture<T>::write_img(const std::string &filename,
                           const bool linear) const {
    // only support Gray and RGB image
    static_assert(std::is_same<T, float>::value ||
                  std::is_same<T, vec3>::value);

    cv::Mat img;
    if constexpr (std::is_same<T, float>::value) {  // float
        img = cv::Mat(height, width, CV_8UC1);
    } else {  // vec3
        img = cv::Mat(height, width, CV_8UC3);
    }

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            T t = truncate_color(at(x, y));
            if (!linear) t = gamma_correction(t, 1.f / 2.2f);
            if constexpr (std::is_same<T, float>::value) {  // float
                img.at<uchar>(y, x) = t * 255.f;
            } else {  // vec3
                img.at<cv::Vec3b>(y, x)[2] = t[0] * 255.f;
                img.at<cv::Vec3b>(y, x)[1] = t[1] * 255.f;
                img.at<cv::Vec3b>(y, x)[0] = t[2] * 255.f;
            }
        }
    }
    cv::imwrite(filename, img);
}

template <typename T>
T Texture<T>::sample(const vec2 &uv) const {
    float x = uv.x() * static_cast<float>(width);
    float y = (1.f - uv.y()) * static_cast<float>(height);

    // truncate uv
    x = std::max(x, 0.5f + EPS);
    x = std::min(x, width - (0.5f + EPS));
    y = std::max(y, 0.5f + EPS);
    y = std::min(y, height - (0.5f + EPS));

    int xl = std::floor(x - 0.5f);
    int xr = xl + 1;
    int yl = std::floor(y - 0.5f);
    int yr = yl + 1;

    float wx = x - (xl + 0.5f);
    float wy = y - (yl + 0.5f);

    T sample_xlyl = at(xl, yl);
    T sample_xlyr = at(xl, yr);
    T sample_xryl = at(xr, yl);
    T sample_xryr = at(xr, yr);

    return (1.f - wy) * ((1.f - wx) * sample_xlyl + wx * sample_xryl) +
           wy * ((1.f - wx) * sample_xlyr + wx * sample_xryr);
}

template <typename T>
float Texture<T>::gamma_correction(const float val, const float gamma) {
    return std::pow(val, gamma);
}

template <typename T>
vec3 Texture<T>::gamma_correction(const vec3 &val, const float gamma) {
    return vec3(gamma_correction(val.x(), gamma),
                gamma_correction(val.y(), gamma),
                gamma_correction(val.z(), gamma));
}

template <typename T>
float Texture<T>::truncate_color(float color) {
    color = std::min(color, 1.f);
    // color = std::max(color, 0.f);
    return color;
}

template <typename T>
vec3 Texture<T>::truncate_color(const vec3 &color) {
    return vec3(truncate_color(color.x()), truncate_color(color.y()),
                truncate_color(color.z()));
}

#endif