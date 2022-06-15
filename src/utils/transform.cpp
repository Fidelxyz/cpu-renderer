#include "utils/transform.hpp"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <cmath>

////////////////////
// NormalTransform
////////////////////

NormalTransform::NormalTransform() { matrix = Eigen::Matrix3f::Identity(); }

void NormalTransform::rotation(const vec3 &angle_deg) {
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

void NormalTransform::scale(const vec3 &factor) {
    mat3 scale_matrix;
    // clang-format off
    scale_matrix << factor.x(), 0, 0,
                    0, factor.y(), 0,
                    0, 0, factor.z();
    // clang-format on

    // Correction for normal transform
    matrix = scale_matrix.transpose().inverse() * matrix;
}

////////////////////
// PositionTransform
////////////////////

PositionTransform::PositionTransform() { matrix = Eigen::Matrix4f::Identity(); }

void PositionTransform::translation(const vec3 &dist) {
    mat4 trans_matrix;
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

void PositionTransform::scale(const vec3 &factor) {
    mat4 scale_matrix;
    // clang-format off
    scale_matrix << factor.x(), 0, 0, 0,
                    0, factor.y(), 0, 0,
                    0, 0, factor.z(), 0,
                    0, 0, 0,          1;
    // clang-format on

    matrix = scale_matrix * matrix;
}

void PositionTransform::world_to_view(const Camera &camera) {
    vec3 left_dir = camera.up_dir.cross(camera.look_dir);

    mat4 view_translation;
    mat4 view_rotation;

    // clang-format off
    view_translation << 1, 0, 0, -camera.pos.x(),
                        0, 1, 0, -camera.pos.y(),
                        0, 0, 1, -camera.pos.z(),
                        0, 0, 0, 1;

    view_rotation << left_dir.x(),        left_dir.y(),        left_dir.z(),        0,
                     camera.up_dir.x(),   camera.up_dir.y(),   camera.up_dir.z(),   0,
                     camera.look_dir.x(), camera.look_dir.y(), camera.look_dir.z(), 0,
                     0,                   0,                   0,                   1;
    // clang-format on

    matrix = view_rotation * view_translation * matrix;
}

void PositionTransform::view_to_projection(const Camera &camera) {
    mat4 projection_persp_to_ortho;
    mat4 projection_ortho_translation;
    mat4 projection_ortho_scale;

    float ortho_height = camera.near_plane * std::tan(camera.fov / 2.f) * 2.f;
    float ortho_width = ortho_height * camera.aspect;
    float ortho_depth = camera.far_plane - camera.near_plane;
    float ortho_center_z = (camera.near_plane + camera.far_plane) / 2.f;

    // clang-format off
    projection_persp_to_ortho << camera.near_plane, 0, 0, 0,
                                 0, camera.near_plane, 0, 0,
                                 0, 0, camera.near_plane + camera.far_plane, -camera.near_plane * camera.far_plane,
                                 0, 0, 1, 0;

    projection_ortho_translation << 1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, -ortho_center_z,
                                    0, 0, 0, 1;

    projection_ortho_scale << 2.f / ortho_width, 0,                  0,                 0,
                              0,                 2.f / ortho_height, 0,                 0,
                              0,                 0,                  2.f / ortho_depth, 0,
                              0,                 0,                  0,                 1;
    // clang-format on

    matrix = projection_ortho_scale * projection_ortho_translation *
             projection_persp_to_ortho * matrix;
}

void PositionTransform::projection_to_screen(const Camera &camera) {
    mat4 screen_transform;
    float screen_scale_x = static_cast<float>(camera.width) / 2.f;
    float screen_scale_y = static_cast<float>(camera.height) / 2.f;

    // clang-format off
    screen_transform << screen_scale_x, 0, 0, screen_scale_x,
                        0, screen_scale_y, 0, screen_scale_y,
                        0, 0, 0.5, 0.5,
                        0, 0, 0, 1;
    // clang-format on

    matrix = screen_transform * matrix;
}