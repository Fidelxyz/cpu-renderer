#include "geometry/triangle.hpp"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <cmath>

#include "global.hpp"

Triangle::Triangle() {}

float Triangle::cross2d(const vec2 &v1, const vec2 &v2) {
    return v1.x() * v2.y() - v2.x() * v1.y();
}

float Triangle::truncate_x_ss(float x, const Camera &camera) {
    x = std::min(x, static_cast<float>(camera.width));
    x = std::max(x, 0.f);
    return x;
}

float Triangle::truncate_y_ss(float y, const Camera &camera) {
    y = std::min(y, static_cast<float>(camera.height));
    y = std::max(y, 0.f);
    return y;
}

float Triangle::truncate_color(float color) {
    color = std::min(color, 1.f);
    color = std::max(color, 0.f);
    return color;
}

vec3 Triangle::truncate_color(const vec3 &color) {
    return vec3(truncate_color(color.x()), truncate_color(color.y()),
                truncate_color(color.z()));
}

bool Triangle::is_inside_ss(const vec3 &w) const {
    return -EPS < w.x() && -EPS < w.y() && -EPS < w.z();
}

vec3 Triangle::normal() const {
    return (vertices[0]->pos - vertices[1]->pos)
        .cross(vertices[0]->pos - vertices[2]->pos)
        .normalized();
}

vec3 Triangle::bary_coord_ss(const vec2 &screen_pos) const {
    auto v1 = vec2(vertices[0]->screen_pos.x(), vertices[0]->screen_pos.y());
    auto v2 = vec2(vertices[1]->screen_pos.x(), vertices[1]->screen_pos.y());
    auto v3 = vec2(vertices[2]->screen_pos.x(), vertices[2]->screen_pos.y());

    float dx1 = v2.x() - v3.x();
    float dy1 = v2.y() - v3.y();
    float det1 = v2.x() * v3.y() - v3.x() * v2.y();

    float w1 = (dy1 * screen_pos.x() - dx1 * screen_pos.y() + det1) /
               (dy1 * v1.x() - dx1 * v1.y() + det1);

    float dx2 = v3.x() - v1.x();
    float dy2 = v3.y() - v1.y();
    float det2 = v3.x() * v1.y() - v1.x() * v3.y();

    float w2 = (dy2 * screen_pos.x() - dx2 * screen_pos.y() + det2) /
               (dy2 * v2.x() - dx2 * v2.y() + det2);

    float w3 = 1.f - w1 - w2;

    return vec3(w1, w2, w3);
}

float Triangle::interpolate_z_ss(const vec3 &bary_coord_ss) const {
    float w1 = bary_coord_ss.x();
    float w2 = bary_coord_ss.y();
    float w3 = bary_coord_ss.z();
    float z1 = vertices[0]->screen_pos.z();
    float z2 = vertices[1]->screen_pos.z();
    float z3 = vertices[2]->screen_pos.z();
    return w1 * z1 + w2 * z2 + w3 * z3;
}

std::tuple<float, float, float> Triangle::corrected_bary_coord(
    const vec3 &bary_coord_ss) const {
    float w1 = vertices[0]->w;
    float w2 = vertices[1]->w;
    float w3 = vertices[2]->w;
    float c1 = bary_coord_ss.x();
    float c2 = bary_coord_ss.y();
    float c3 = bary_coord_ss.z();
    float w = c1 / w1 + c2 / w2 + c3 / w3;
    return std::make_tuple(c1 / w1 / w, c2 / w2 / w, c3 / w3 / w);
}

template <typename T>
T Triangle::interpolate(const std::tuple<T, T, T> &vals,
                        const std::tuple<float, float, float> &weights) {
    auto [w1, w2, w3] = weights;
    auto [v1, v2, v3] = vals;
    return w1 * v1 + w2 * v2 + w3 * v3;
}

vec3 Triangle::shade(size_t pixel_x, size_t pixel_y, const vec3 &bary_coord,
                     FragmentShader *fragment_shader) const {
    // perspective-corrected interpolate
    auto w = corrected_bary_coord(bary_coord);

    vec3 pos = interpolate(
        std::make_tuple(vertices[0]->pos, vertices[1]->pos, vertices[2]->pos),
        w);

    vec3 normal =
        normals.empty()
            ? this->normal()
            : interpolate(
                  std::make_tuple(*normals[0], *normals[1], *normals[2]), w);

    vec2 uv = texcoords.empty()
                  ? vec2(0, 0)
                  : interpolate(std::make_tuple(*texcoords[0], *texcoords[1],
                                                *texcoords[2]),
                                w);

    return truncate_color(
        fragment_shader->shade(pos, normal, uv, material.get()));
}

void Triangle::rasterize(Texture<vec3> *frame_buffer, Texture<float> *z_buffer,
                         FragmentShader *fragment_shader,
                         const Camera &camera) const {
    auto v1 = vertices[0];
    auto v2 = vertices[1];
    auto v3 = vertices[2];
    int min_x = truncate_x_ss(
        std::floor(std::min(
            {v1->screen_pos.x(), v2->screen_pos.x(), v3->screen_pos.x()})),
        camera);
    int min_y = truncate_y_ss(
        std::floor(std::min(
            {v1->screen_pos.y(), v2->screen_pos.y(), v3->screen_pos.y()})),
        camera);
    int max_x = truncate_x_ss(
        std::ceil(std::max(
            {v1->screen_pos.x(), v2->screen_pos.x(), v3->screen_pos.x()})),
        camera);
    int max_y = truncate_y_ss(
        std::ceil(std::max(
            {v1->screen_pos.y(), v2->screen_pos.y(), v3->screen_pos.y()})),
        camera);

    vec3 bary_coord_init = bary_coord_ss(vec2(min_x + 0.5f, min_y + 0.5f));
    vec3 bary_coord_dx =
        bary_coord_ss(vec2(min_x + 1.5f, min_y + 0.5f)) - bary_coord_init;
    vec3 bary_coord_dy =
        bary_coord_ss(vec2(min_x + 0.5f, min_y + 1.5f)) - bary_coord_init;

    vec3 bary_coord_y = bary_coord_init;
    for (int pixel_y = min_y; pixel_y < max_y; pixel_y++) {
        vec3 bary_coord_x = bary_coord_y;
        for (int pixel_x = min_x; pixel_x < max_x; pixel_x++) {
            if (is_inside_ss(bary_coord_x)) {
                // screen space interpolate
                float z = interpolate_z_ss(bary_coord_x);
                if (0.f < z && z < z_buffer->at(pixel_x, pixel_y)) {
                    z_buffer->at(pixel_x, pixel_y) = z;
                    vec3 shading =
                        shade(pixel_x, pixel_y, bary_coord_x, fragment_shader);

                    // write into frame buffer
                    frame_buffer->at(pixel_x, pixel_y) = std::move(shading);
                }
            }
            bary_coord_x += bary_coord_dx;
        }
        bary_coord_y += bary_coord_dy;
    }
}