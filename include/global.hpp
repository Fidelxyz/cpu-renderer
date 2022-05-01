#ifndef GLOBAL_H
#define GLOBAL_H

#include <Eigen/Core>
#include <cmath>

using vec2 = Eigen::Vector2f;
using vec3 = Eigen::Vector3f;
using vec4 = Eigen::Vector4f;
using mat2 = Eigen::Matrix2f;
using mat3 = Eigen::Matrix3f;
using mat4 = Eigen::Matrix4f;

using ivec2 = Eigen::Vector2i;
using ivec3 = Eigen::Vector3i;
using ivec4 = Eigen::Vector4i;

const float EPS = 1e-8;

namespace camera {
const vec3 POS = vec3(0.1, -1, -0.25);
const vec3 UP_DIR = vec3(0, 1, 0).normalized();
const vec3 LOOK_DIR = vec3(0, 0, -1).normalized();
const float FOV = 60.f * M_PI / 180.f;
const float NEAR_PLANE = 0.1;
const float FAR_PLANE = 500.0;
const int WIDTH = 640;
const int HEIGHT = 480;
}  // namespace camera

#endif