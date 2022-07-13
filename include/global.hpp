#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#include <Eigen/Core>

using vec2 = Eigen::Vector2f;
using vec3 = Eigen::Vector3f;
using vec4 = Eigen::Vector4f;

template <size_t NDims>
using vec = Eigen::Matrix<float, NDims, 1>;

using mat2 = Eigen::Matrix2f;
using mat3 = Eigen::Matrix3f;
using mat4 = Eigen::Matrix4f;

using vec2d = Eigen::Vector2d;
using vec3d = Eigen::Vector3d;
using vec4d = Eigen::Vector4d;

using mat2d = Eigen::Matrix2d;
using mat3d = Eigen::Matrix3d;
using mat4d = Eigen::Matrix4d;

using ivec2 = Eigen::Vector2i;
using ivec3 = Eigen::Vector3i;
using ivec4 = Eigen::Vector4i;

const float EPS = 1e-4;

#endif