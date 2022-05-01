#include "camera.hpp"

Camera::Camera(const vec3 &pos, const vec3 &look_dir, const vec3 &up_dir,
               const float fov, const float near_plane, const float far_plane,
               const int width, const int height) {
    this->pos = pos;
    this->look_dir = look_dir;
    this->up_dir = up_dir;
    this->fov = fov;
    this->near_plane = near_plane;
    this->far_plane = far_plane;
    this->width = width;
    this->height = height;
    this->aspect = static_cast<float>(width) / static_cast<float>(height);
}