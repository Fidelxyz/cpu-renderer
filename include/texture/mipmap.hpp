#pragma once
#ifndef MIPMAP_H
#define MIPMAP_H

#include <cmath>
#include <memory>

#include "global.hpp"
#include "texture/texture.hpp"

namespace mipmap {
const float LOD_SAMPLE_DELTA = 0.1;
const int MIPMAP_LEVEL = 4;
}  // namespace mipmap

template <typename T>
class Mipmap {
   public:
    Mipmap(const std::shared_ptr<Texture<T>> &src);

    T sample(const vec2 &uv, const vec2 &duv) const;

   private:
    std::shared_ptr<Texture<T>> texture[mipmap::MIPMAP_LEVEL]
                                       [mipmap::MIPMAP_LEVEL];

    std::tuple<float, float> calc_lod(const vec2 &duv) const;
};

template <typename T>
Mipmap<T>::Mipmap(const std::shared_ptr<Texture<T>> &src) {
    texture[0][0] = src;
    for (size_t lod_y = 0; lod_y < mipmap::MIPMAP_LEVEL; lod_y++) {
        if (lod_y != 0) {
            texture[lod_y][0] =
                std::make_shared<Texture<T>>(texture[lod_y - 1][0]->width,
                                             texture[lod_y - 1][0]->height / 2);

#pragma omp parallel for
            for (size_t y = 0; y < texture[lod_y][0]->height; y++) {
                for (size_t x = 0; x < texture[lod_y][0]->width; x++) {
                    texture[lod_y][0]->at(x, y) =
                        (texture[lod_y - 1][0]->at(x, y * 2) +
                         texture[lod_y - 1][0]->at(x, y * 2 + 1)) /
                        2;
                }
            }
        }
        for (size_t lod_x = 1; lod_x < mipmap::MIPMAP_LEVEL; lod_x++) {
            texture[lod_y][lod_x] = std::make_shared<Texture<T>>(
                texture[lod_y][lod_x - 1]->width / 2,
                texture[lod_y][lod_x - 1]->height);

#pragma omp parallel for
            for (size_t y = 0; y < texture[lod_y][lod_x]->height; y++) {
                for (size_t x = 0; x < texture[lod_y][lod_x]->width; x++) {
                    texture[lod_y][lod_x]->at(x, y) =
                        (texture[lod_y][lod_x - 1]->at(x * 2, y) +
                         texture[lod_y][lod_x - 1]->at(x * 2 + 1, y)) /
                        2;
                }
            }
        }
    }
}

template <typename T>
std::tuple<float, float> Mipmap<T>::calc_lod(const vec2 &duv) const {
    float lod_x = std::log2(duv.x() * texture[0][0]->width);
    float lod_y = std::log2(duv.y() * texture[0][0]->height);
    return std::make_tuple(lod_x, lod_y);
}

template <typename T>
T Mipmap<T>::sample(const vec2 &uv, const vec2 &duv) const {
    if constexpr (mipmap::MIPMAP_LEVEL == 1) {
        return texture[0][0]->sample(uv);
    }

    auto [lod_x, lod_y] = calc_lod(duv);

    // truncate lod
    lod_x = std::max(lod_x, EPS);
    lod_x = std::min(lod_x, mipmap::MIPMAP_LEVEL - (1.f + EPS));
    lod_y = std::max(lod_y, EPS);
    lod_y = std::min(lod_y, mipmap::MIPMAP_LEVEL - (1.f + EPS));

    int xl = std::floor(lod_x);
    int xr = xl + 1;
    int yl = std::floor(lod_y);
    int yr = yl + 1;

    float wx = lod_x - static_cast<float>(xl);
    float wy = lod_y - static_cast<float>(yl);

    return (1.f - wy) * ((1.f - wx) * texture[yl][xl]->sample(uv) +
                         wx * texture[yl][xr]->sample(uv)) +
           wy * ((1.f - wx) * texture[yr][xl]->sample(uv) +
                 wx * texture[yr][xr]->sample(uv));
}

#endif