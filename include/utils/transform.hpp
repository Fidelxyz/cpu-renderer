#pragma once
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "global.hpp"
#include "scene/camera.hpp"

class DirectionTransform {
   protected:
    mat3 matrix;

   public:
    DirectionTransform();

    vec3 transform(const vec3 &pos) const;
    void rotation(const vec3 &angle_deg);
};

class NormalTransform : public DirectionTransform {
   public:
    void scale(const vec3 &factor);
};

class PositionTransform {
   protected:
    mat4 matrix;

   public:
    PositionTransform();

    vec4 transform(const vec4 &pos) const;

    void translation(const vec3 &dist);
    void rotation(const vec3 &angle_deg);
    void scale(const vec3 &factor);

    void world_to_view(const Camera &camera);
    void view_to_ndc(const Camera &camera);
    void ndc_to_screen(const Camera &camera);
};

#endif