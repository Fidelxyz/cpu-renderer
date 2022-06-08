#pragma once
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "global.hpp"

class Transform3 {
   private:
    mat3 matrix;

   public:
    Transform3();

    template <typename T>
    T transform(const T &pos);

    void rotation(const vec3 &angle_deg);
    void scale(const vec3 &factor);
};

class Transform4 {
   private:
    mat4 matrix;

   public:
    Transform4();

    template <typename T>
    T transform(const T &pos);

    void translation(const vec3 &dist);
    void rotation(const vec3 &angle_deg);
    void scale(const vec3 &factor);
};

template <typename T>
T Transform3::transform(const T &pos) {
    return matrix * pos;
}

template <typename T>
T Transform4::transform(const T &pos) {
    return matrix * pos;
}

#endif