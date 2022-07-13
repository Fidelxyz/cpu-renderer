#include "utils/transform.hpp"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <cmath>

////////////////////
// NormalTransform
////////////////////

NormalTransform::NormalTransform() { matrix = Eigen::Matrix3d::Identity(); }

vec3d NormalTransform::transform(const vec3d &pos) const {
    return matrix * pos;
}

void NormalTransform::rotation(const vec3 &angle_deg) {
    vec3 angle_arc = angle_deg * M_PI / 180.f;
    float sin_x = std::sin(angle_arc.x());
    float sin_y = std::sin(angle_arc.y());
    float sin_z = std::sin(angle_arc.z());
    float cos_x = std::cos(angle_arc.x());
    float cos_y = std::cos(angle_arc.y());
    float cos_z = std::cos(angle_arc.z());

    mat3d rot_x, rot_y, rot_z;
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
    mat3d rot_matrix = rot_z * rot_y * rot_x;

    matrix = rot_matrix * matrix;
}

void NormalTransform::scale(const vec3 &factor) {
    mat3d scale_matrix;
    // clang-format off
    scale_matrix << factor.x(), 0, 0,
                    0, factor.y(), 0,
                    0, 0, factor.z();
    // clang-format on

    // Correction for normal transform
    matrix = scale_matrix.inverse().transpose() * matrix;
}

////////////////////
// PositionTransform
////////////////////

PositionTransform::PositionTransform() { matrix = Eigen::Matrix4d::Identity(); }

vec4d PositionTransform::transform(const vec4d &pos) const {
    return matrix * pos;
}

void PositionTransform::translation(const vec3 &dist) {
    mat4d trans_matrix;
    // clang-format off
    trans_matrix << 1, 0, 0, dist.x(),
                    0, 1, 0, dist.y(),
                    0, 0, 1, dist.z(),
                    0, 0, 0, 1;
    // clang-format on

    matrix = trans_matrix * matrix;
}

void PositionTransform::rotation(const vec3 &angle_deg) {
    vec3 angle_arc = angle_deg * M_PI / 180.f;
    float sin_x = std::sin(angle_arc.x());
    float sin_y = std::sin(angle_arc.y());
    float sin_z = std::sin(angle_arc.z());
    float cos_x = std::cos(angle_arc.x());
    float cos_y = std::cos(angle_arc.y());
    float cos_z = std::cos(angle_arc.z());

    mat4d rot_x, rot_y, rot_z;
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
    mat4d rot_matrix = rot_z * rot_y * rot_x;

    matrix = rot_matrix * matrix;
}

void PositionTransform::scale(const vec3 &factor) {
    mat4d scale_matrix;
    // clang-format off
    scale_matrix << factor.x(), 0, 0, 0,
                    0, factor.y(), 0, 0,
                    0, 0, factor.z(), 0,
                    0, 0, 0,          1;
    // clang-format on

    matrix = scale_matrix * matrix;
}

void PositionTransform::world_to_view(const Camera &camera) {
    // orthogonalization
    vec3 look_dir = camera.look_dir;
    vec3 up_dir =
        (camera.up_dir - camera.up_dir.dot(look_dir) * look_dir).normalized();
    vec3 left_dir = camera.up_dir.cross(camera.look_dir);

    mat4d view_translation;
    mat4d view_rotation;

    // clang-format off
    view_translation << 1, 0, 0, -camera.pos.x(),
                        0, 1, 0, -camera.pos.y(),
                        0, 0, 1, -camera.pos.z(),
                        0, 0, 0, 1;

    view_rotation << left_dir.x(), left_dir.y(), left_dir.z(), 0,
                     up_dir.x(),   up_dir.y(),   up_dir.z(),   0,
                     look_dir.x(), look_dir.y(), look_dir.z(), 0,
                     0,            0,            0,            1;
    // clang-format on

    matrix = view_rotation * view_translation * matrix;
}

void PositionTransform::view_to_ndc(const Camera &camera) {
    float t = camera.near_plane * std::tan(camera.fov / 2.f);
    float r = t * camera.aspect;

    float n = camera.near_plane;
    float f = camera.far_plane;

    mat4d projection_matrix;

    // clang-format off
    projection_matrix << n / r, 0, 0, 0,
                         0, n / t, 0, 0,
                         0, 0, -(f + n) / (f - n), -2.f * f * n / (f - n),
                         0, 0, -1, 0;
    // clang-format on

    matrix = projection_matrix * matrix;
}

void PositionTransform::ndc_to_screen(const Camera &camera) {
    // x: [-1, 1] -> [0, width]
    // y: [-1, 1] -> [0, height]
    // z: [-1, 1] -> [0, 1]

    float screen_scale_x = static_cast<float>(camera.width) / 2.f;
    float screen_scale_y = static_cast<float>(camera.height) / 2.f;

    mat4d screen_transform;

    // clang-format off
    screen_transform << screen_scale_x, 0, 0, screen_scale_x,
                        0, -screen_scale_y, 0, screen_scale_y,
                        0, 0, 0.5, 0.5,
                        0, 0, 0, 1;
    // clang-format on

    matrix = screen_transform * matrix;
}

/*
z = z * (n + f) - n * f
z = z - (n + f) / 2
z = z * 2 / (f - n)
z = z * 0.5 + 0.5

w = z

z' = ((z * (n + f) - n * f - (n + f) / 2) * 2 / (f - n) / 2 + 0.5) / z
z' = (n + f) / (f - n) - (n * f + n) / (f - n) / z
z = (n * f + n) / (n + f + (n - f) * z')

*/