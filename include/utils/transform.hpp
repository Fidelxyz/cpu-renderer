#pragma once
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "global.hpp"
#include "scene/camera.hpp"

class NormalTransform {
   private:
    mat3d matrix;

   public:
    NormalTransform();

    vec3d transform(const vec3d &pos) const;

    void rotation(const vec3 &angle_deg);
    void scale(const vec3 &factor);
};

class PositionTransform {
   private:
    mat4d matrix;

   public:
    PositionTransform();

    vec4d transform(const vec4d &pos) const;

    void translation(const vec3 &dist);
    void rotation(const vec3 &angle_deg);
    void scale(const vec3 &factor);

    void world_to_view(const Camera &camera);
    void view_to_ndc(const Camera &camera);
    void ndc_to_screen(const Camera &camera);
};

#endif