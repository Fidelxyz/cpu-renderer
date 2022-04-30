#ifndef CAMERA_H
#define CAMERA_H

#include "global.hpp"

class Camera {
   public:
    vec3 pos;
    vec3 look_dir;
    vec3 up_dir;
    float fov;
    float aspect;
    float near_plane;
    float far_plane;

    Camera(const vec3 pos, const vec3 look_dir, const vec3 up_dir,
           const float fov, const float aspect, const float near_plane,
           const float far_plane);
};

#endif