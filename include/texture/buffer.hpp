#pragma once
#ifndef BUFFER_H
#define BUFFER_H

#include <omp.h>

#include <memory>

#include "effects/msaa.hpp"
#include "texture/texture.hpp"

template <typename T>
using msaa_texture_t = Texture<std::array<T, msaa::MSAA_LEVEL>>;

using frame_buffer_t = msaa_texture_t<vec3>;
using z_buffer_t = msaa_texture_t<float>;

class Buffer {
   public:
    std::shared_ptr<Texture<omp_lock_t>> mutex = nullptr;

    std::shared_ptr<frame_buffer_t> frame_buffer = nullptr;
    std::shared_ptr<z_buffer_t> z_buffer = nullptr;

    std::shared_ptr<msaa_texture_t<vec3>> pos_buffer = nullptr;
    std::shared_ptr<msaa_texture_t<vec3>> normal_buffer = nullptr;
    std::shared_ptr<Texture<bool>> full_covered = nullptr;
};

#endif