#include "shader/vertex_shader.hpp"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <cmath>

VertexShader::VertexShader(const Camera &camera) {
    vec3 left_dir = camera.up_dir.cross(camera.look_dir);

    // View transform

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

    mat4 view_transform = view_rotation * view_translation;

    // Projection transformation

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

    mat4 projection_transform = projection_ortho_scale *
                                projection_ortho_translation *
                                projection_persp_to_ortho;

    // Screen transform

    mat4 screen_transform;
    float screen_scale_x = static_cast<float>(camera.width) / 2.f;
    float screen_scale_y = static_cast<float>(camera.height) / 2.f;

    // clang-format off
    screen_transform << screen_scale_x, 0, 0, screen_scale_x,
                        0, screen_scale_y, 0, screen_scale_y,
                        0, 0, 0.5, 0.5,
                        0, 0, 0, 1;
    // clang-format on

    transform = screen_transform * projection_transform * view_transform;
}

void VertexShader::shade(Vertex *vertex, const Object &object) {
    vec4 screen_pos =
        transform * object.model_transform *
        vec4(vertex->pos.x(), vertex->pos.y(), vertex->pos.z(), 1);
    vertex->screen_pos =
        vec3(screen_pos.x(), screen_pos.y(), screen_pos.z()) / screen_pos.w();
}