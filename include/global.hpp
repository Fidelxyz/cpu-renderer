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

const float EPS = 1e-5;

#endif