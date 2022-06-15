#pragma once
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "global.hpp"
#include "scene/camera.hpp"

class NormalTransform {
   private:
    mat3 matrix;

   public:
    NormalTransform();

    template <typename T>
    T transform(const T &pos) const;

    void rotation(const vec3 &angle_deg);
    void scale(const vec3 &factor);
};

class PositionTransform {
   private:
    mat4 matrix;

   public:
    PositionTransform();

    template <typename T>
    T transform(const T &pos) const;

    void translation(const vec3 &dist);
    void rotation(const vec3 &angle_deg);
    void scale(const vec3 &factor);

    void world_to_view(const Camera &camera);
    void view_to_projection(const Camera &camera);
    void projection_to_screen(const Camera &camera);
};

template <typename T>
T NormalTransform::transform(const T &pos) const {
    return matrix * pos;
}

template <typename T>
T PositionTransform::transform(const T &pos) const {
    return matrix * pos;
}

#endif