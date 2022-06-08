#include "utils/transform.hpp"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <cmath>

////////////////////
// Transform3
////////////////////

Transform3::Transform3() { matrix = Eigen::Matrix3f::Identity(); }

void Transform3::rotation(const vec3 &angle_deg) {
    vec3 angle_arc = angle_deg * M_PI / 180.f;
    float sin_x = std::sin(angle_arc.x());
    float sin_y = std::sin(angle_arc.y());
    float sin_z = std::sin(angle_arc.z());
    float cos_x = std::cos(angle_arc.x());
    float cos_y = std::cos(angle_arc.y());
    float cos_z = std::cos(angle_arc.z());

    mat3 rot_x, rot_y, rot_z;
    // clang-format off
    rot_x << 1, 0,     0,    
             0, cos_x, -sin_x,
             0, sin_x, cos_x;

    rot_y << cos_y,  0, sin_y,
             0,      1, 0,    
             -sin_y, 0, cos_y;

    rot_z << cos_z, -sin_z, 0,
             sin_z, cos_z,  0,
             0,     0,      1;
    // clang-format on
    mat3 rot_matrix = rot_z * rot_y * rot_x;

    matrix = rot_matrix * matrix;
}

void Transform3::scale(const vec3 &factor) {
    mat3 scale_matrix;
    // clang-format off
    scale_matrix << factor.x(), 0, 0,
                    0, factor.y(), 0,
                    0, 0, factor.z();
    // clang-format on

    matrix = scale_matrix * matrix;
}

////////////////////
// Transform4
////////////////////

Transform4::Transform4() { matrix = Eigen::Matrix4f::Identity(); }

void Transform4::translation(const vec3 &dist) {
    mat4 trans_matrix;
    // clang-format off
    trans_matrix << 1, 0, 0, dist.x(),
                    0, 1, 0, dist.y(),
                    0, 0, 1, dist.z(),
                    0, 0, 0, 1;
    // clang-format on

    matrix = trans_matrix * matrix;
}

void Transform4::rotation(const vec3 &angle_deg) {
    vec3 angle_arc = angle_deg * M_PI / 180.f;
    float sin_x = std::sin(angle_arc.x());
    float sin_y = std::sin(angle_arc.y());
    float sin_z = std::sin(angle_arc.z());
    float cos_x = std::cos(angle_arc.x());
    float cos_y = std::cos(angle_arc.y());
    float cos_z = std::cos(angle_arc.z());

    mat4 rot_x, rot_y, rot_z;
    // clang-format off
    rot_x << 1, 0,     0,      0,
             0, cos_x, -sin_x, 0,
             0, sin_x, cos_x,  0,
             0, 0,     0,      1;

    rot_y << cos_y,  0, sin_y, 0,
             0,      1, 0,     0,
             -sin_y, 0, cos_y, 0,
             0,      0, 0,     1;

    rot_z << cos_z, -sin_z, 0, 0,
             sin_z, cos_z,  0, 0,
             0,     0,      1, 0,
             0,     0,      0, 1;
    // clang-format on
    mat4 rot_matrix = rot_z * rot_y * rot_x;

    matrix = rot_matrix * matrix;
}

void Transform4::scale(const vec3 &factor) {
    mat4 scale_matrix;
    // clang-format off
    scale_matrix << factor.x(), 0, 0, 0,
                    0, factor.y(), 0, 0,
                    0, 0, factor.z(), 0,
                    0, 0, 0,          1;
    // clang-format on

    matrix = scale_matrix * matrix;
}