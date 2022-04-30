#include "camera.hpp"

Camera::Camera(const vec3 pos, const vec3 look_dir, const vec3 pos_dir,
               const float fov, const float aspect, const float near_plane,
               const float far_plane) {
    this->pos = pos;
    this->look_dir = look_dir;
    this->up_dir = up_dir;
    this->fov = fov;
    this->aspect = aspect;
    this->near_plane = near_plane;
    this->far_plane = far_plane;
}