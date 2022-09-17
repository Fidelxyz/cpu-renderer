#include "scene/camera.hpp"

#include "utils/transform.hpp"

Camera::Camera() {}

Camera::Camera(const vec3 &pos, const vec3 &rotation, const float fov,
               const float near_plane, const float far_plane, const int width,
               const int height) {
    this->pos = pos;
    this->fov = fov;
    this->near_plane = near_plane;
    this->far_plane = far_plane;
    this->width = width;
    this->height = height;
    this->aspect = static_cast<float>(width) / static_cast<float>(height);

    DirectionTransform direction_transform;
    direction_transform.rotation(rotation);
    this->look_dir = direction_transform
                         .transform(vec3(0, 0, -1))  // look at -Z
                         .normalized();
    this->up_dir = direction_transform
                       .transform(vec3(0, 1, 0))  // up to Y
                       .normalized();
}