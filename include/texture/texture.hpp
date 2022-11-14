#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <Eigen/Core>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <string>

#include "global.hpp"
#include "utils/functions.hpp"
#include "utils/gamma_correction.hpp"

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
    Texture(const Texture &src);
    ~Texture();

    T *begin() const;
    T *end() const;

    Texture<T> &operator=(Texture<T> &&src);

    inline T &at(const size_t x, const size_t y) const;
    inline T &at(const size_t index) const;
    inline T &operator[](const size_t index) const;

    inline bool is_null() const;

    // re-allowcate space
    void allowcate(const size_t width, const size_t height);

    void read_img(const std::string &filename, const bool linear);
    void write_img(const std::string &filename, const bool linear) const;

    void read_alpha(const std::string &filename);

    T sample(const vec2 &uv) const;
    T sample_no_repeat(const vec2 &uv) const;
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
Texture<T>::Texture(const Texture &src) : Texture(src.width, src.height) {
#pragma omp parallel for
    for (size_t i = 0; i < width * height; i++) {
        this->data[i] = src.data[i];
    }
}

template <typename T>
Texture<T>::~Texture() {
    delete[] data;
}

template <typename T>
T *Texture<T>::begin() const {
    return data;
}

template <typename T>
T *Texture<T>::end() const {
    return data + width * height;
}

template <typename T>
Texture<T> &Texture<T>::operator=(Texture &&src) {
    width = std::move(src.width);
    height = std::move(src.height);
    delete[] data;
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
void Texture<T>::allowcate(const size_t width, const size_t height) {
    if (this->width != width || this->height != height) {
        this->width = width;
        this->height = height;
        delete[] data;
        data = new T[width * height];
    }
}

template <typename T>
void Texture<T>::read_img(const std::string &filename, const bool linear) {
    // only support Gray and RGB image
    static_assert(std::is_same_v<T, float> || std::is_same_v<T, vec3>);

    cv::Mat img;
    if constexpr (std::is_same_v<T, float>) {  // float
        img = cv::imread(filename, cv::IMREAD_GRAYSCALE);
    } else {  // vec3
        img = cv::imread(filename, cv::IMREAD_COLOR);
    }

    if (img.rows == 0 || img.cols == 0) {
        std::cerr << "Load image failed." << std::endl;
        return;
    }

    //
    if (!linear) {
        GammaCorrection::to_linear(&img);
    }

    // re-allowcate space
    allowcate(img.rows, img.cols);

    cv::parallel_for_(
        cv::Range(0, width * height), [&](const cv::Range &range) {
            for (int r = range.start; r < range.end; r++) {
                if constexpr (std::is_same_v<T, float>) {  // float
                    at(r) = img.at<uchar>(r) / 255.f;
                } else {  // vec3
                    at(r)[0] = img.at<cv::Vec3b>(r)[2] / 255.f;
                    at(r)[1] = img.at<cv::Vec3b>(r)[1] / 255.f;
                    at(r)[2] = img.at<cv::Vec3b>(r)[0] / 255.f;
                }
            }
        });
}

template <typename T>
void Texture<T>::write_img(const std::string &filename,
                           const bool linear) const {
    // only support Gray and RGB image
    static_assert(std::is_same_v<T, float> || std::is_same_v<T, vec3>);

    cv::Mat img;
    if constexpr (std::is_same_v<T, float>) {  // float
        img = cv::Mat(height, width, CV_8UC1);
    } else {  // vec3
        img = cv::Mat(height, width, CV_8UC3);
    }

    cv::parallel_for_(
        cv::Range(0, width * height), [&](const cv::Range &range) {
            for (int r = range.start; r < range.end; r++) {
                T t = at(r);
                if (!linear) t = gamma_correction(t, 1.f / 2.2f);
                t *= 255.f;
                if constexpr (std::is_same_v<T, float>) {  // float
                    img.at<uchar>(r) = cv::saturate_cast<uchar>(t);
                } else {  // vec3
                    img.at<cv::Vec3b>(r)[2] = cv::saturate_cast<uchar>(t[0]);
                    img.at<cv::Vec3b>(r)[1] = cv::saturate_cast<uchar>(t[1]);
                    img.at<cv::Vec3b>(r)[0] = cv::saturate_cast<uchar>(t[2]);
                }
            }
        });

    cv::imwrite(filename, img);
}

template <typename T>
void Texture<T>::read_alpha(const std::string &filename) {
    static_assert(std::is_same_v<T, float>);

    cv::Mat img = cv::imread(filename, cv::IMREAD_UNCHANGED);

    if (img.channels() != 4) {
        std::cerr << "Load alpha failed: no alpha channel." << std::endl;
        return;
    }

    if (img.rows == 0 || img.cols == 0) {
        std::cerr << "Load image failed." << std::endl;
        return;
    }

    allowcate(img.rows, img.cols);

    cv::parallel_for_(cv::Range(0, width * height),
                      [&](const cv::Range &range) {
                          for (int r = range.start; r < range.end; r++) {
                              at(r) = img.at<cv::Vec4b>(r)[3] / 255.f;
                          }
                      });
}

template <typename T>
T Texture<T>::sample_no_repeat(const vec2 &uv) const {
    // (x + 0.5, y + 0.5) = (u, v)
    float x = uv.x() * static_cast<float>(width) - 0.5f;
    float y = (1.f - uv.y()) * static_cast<float>(height) - 0.5f;

    // truncate uv
    x = std::max(x, EPS);
    x = std::min(x, width - 1.f - EPS);
    y = std::max(y, EPS);
    y = std::min(y, height - 1.f - EPS);

    int xl = std::floor(x);
    int xr = xl + 1;
    int yl = std::floor(y);
    int yr = yl + 1;

    float wx = x - xl;
    float wy = y - yl;

    T sample_xlyl = at(xl, yl);
    T sample_xlyr = at(xl, yr);
    T sample_xryl = at(xr, yl);
    T sample_xryr = at(xr, yr);

    return (1.f - wy) * ((1.f - wx) * sample_xlyl + wx * sample_xryl) +
           wy * ((1.f - wx) * sample_xlyr + wx * sample_xryr);
}

template <typename T>
T Texture<T>::sample(const vec2 &uv) const {
    return sample_no_repeat(
        vec2(uv.x() - std::floor(uv.x()), uv.y() - std::floor(uv.y())));
}

#endif