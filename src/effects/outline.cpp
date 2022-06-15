#include "effects/outline.hpp"

#include <cmath>
#include <memory>
#include <vector>

#include "geometry/shape.hpp"
#include "global.hpp"
#include "light/light.hpp"
#include "utils/transform.hpp"

// Object outline::outline_geometry(const Object &object, const Camera &camera) {
//     auto t_object = Object();
//     t_object.vertices.reserve(object.vertices.size());
//     t_object.normals.reserve(object.normals.size());
//     t_object.shapes.reserve(object.shapes.size());
//     for (auto &shape : object.shapes) {
//         auto t_shape = Shape();
//         t_shape.triangles.reserve(shape.triangles.size());
//         for (auto &triangle : shape.triangles) {
//             auto t_triangle = Triangle();
//             t_triangle.vertices.reserve(3);
//             t_triangle.normals.reserve(3);
//             for (size_t i = 0; i < 3; i++) {
//                 auto t_vertex = std::make_shared<Vertex>();
//                 vec3 view_vec = triangle.vertices[i]->pos - camera.pos;
//                 t_vertex->pos = triangle.vertices[i]->pos +
//                                 (*triangle.normals[i]) * OUTLINE_WIDTH *
//                                     std::tanh(view_vec.norm() / 100.f) +
//                                 view_vec.normalized() * EPS * 50.f;
//                 t_triangle.vertices.push_back(t_vertex);
//                 t_object.vertices.push_back(std::move(t_vertex));

//                 t_triangle.normals.push_back(triangle.normals[i]);
//                 t_object.normals.push_back(triangle.normals[i]);
//             }
//             t_shape.triangles.emplace_back(std::move(t_triangle));
//         }
//         t_object.shapes.emplace_back(std::move(t_shape));
//     }
//     return t_object;
// }

outline::OutlineVertexShader::OutlineVertexShader(const Camera &camera) {
    camera_pos = camera.pos;
}

void outline::OutlineVertexShader::shade(Vertex *vertex) {
    vec3 view_vec = vertex->pos - camera_pos;
    vertex->pos +=
        vertex->normal * OUTLINE_WIDTH * std::tanh(view_vec.norm()) +
        view_vec.normalized() * EPS * 50.f;
}

vec3 outline::OutlineFragmentShader::shade(const vec3 &pos, const vec3 &normal,
                                           const vec2 &uv, const vec2 &duv,
                                           Material *material) {
    return OUTLINE_COLOR;
}