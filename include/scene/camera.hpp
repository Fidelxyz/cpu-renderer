#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "global.hpp"

class Camera {
   public:
    vec3 pos;
    vec3 look_dir;
    vec3 up_dir;
    float fov;
    float near_plane;
    float far_plane;
    int width;
    int height;
    float aspect;

    Camera();
    Camera(const vec3 &pos, const vec3 &rotation, const float fov,
           const float near_plane, const float far_plane, const int width,
           const int height);
};

#endif