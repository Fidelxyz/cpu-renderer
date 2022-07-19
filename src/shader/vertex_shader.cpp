#include "shader/vertex_shader.hpp"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <cmath>

VertexShader::VertexShader() {}

VertexShader::VertexShader(const Camera &camera) {
    position_transform.world_to_view(camera);
    position_transform.view_to_ndc(camera);
    position_transform.ndc_to_screen(camera);
}

void VertexShader::shade(Vertex *vertex) {
    vec4 screen_pos = position_transform.transform(
        vec4(vertex->pos.x(), vertex->pos.y(), vertex->pos.z(), 1));
    vertex->screen_pos =
        (vec3(screen_pos.x(), screen_pos.y(), screen_pos.z()) /
         screen_pos.w());
    vertex->w = screen_pos.w();
}