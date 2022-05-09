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

bool Triangle::is_inside_ss(const vec3 &w) const {
    return 0.f <= w.x() && 0.f <= w.y() && 0.f <= w.z();
}

vec3 Triangle::normal() const {
    return (vertices[0]->pos - vertices[1]->pos)
        .cross(vertices[0]->pos - vertices[2]->pos)
        .normalized();
}

template <typename T>
T Triangle::interpolate(const std::tuple<T, T, T> &vals,
                        const std::tuple<float, float, float> &weights) {
    auto [w1, w2, w3] = weights;
    auto [v1, v2, v3] = vals;
    return w1 * v1 + w2 * v2 + w3 * v3;
}

vec3 Triangle::interpolate_weights_ss(const vec2 &screen_pos) const {
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

float Triangle::interpolate_z(
    const std::tuple<float, float, float> &weights_ss) const {
    auto [w1, w2, w3] = weights_ss;
    float z1 = vertices[0]->screen_pos.z();
    float z2 = vertices[1]->screen_pos.z();
    float z3 = vertices[2]->screen_pos.z();
    return 1.f / (w1 / z1 + w2 / z2 + w3 / z3);
}

std::tuple<float, float, float> Triangle::interpolate_weights(
    const std::tuple<float, float, float> &weights_ss, const float z) const {
    auto [w1, w2, w3] = weights_ss;
    float z1 = vertices[0]->screen_pos.z();
    float z2 = vertices[1]->screen_pos.z();
    float z3 = vertices[2]->screen_pos.z();
    return std::make_tuple(z * z1 * w1, z * z2 * w2, z * z3 * w3);
}

void Triangle::rasterize(Buffer<float> *frame_buffer, Buffer<float> *z_buffer,
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

    vec3 bary_coord_init =
        interpolate_weights_ss(vec2(min_x + 0.5f, min_y + 0.5f));
    vec3 bary_coord_dx =
        interpolate_weights_ss(vec2(min_x + 1.5f, min_y + 0.5f)) -
        bary_coord_init;
    vec3 bary_coord_dy =
        interpolate_weights_ss(vec2(min_x + 0.5f, min_y + 1.5f)) -
        bary_coord_init;

    vec3 bary_coord_y = bary_coord_init;
    for (int pixel_y = min_y; pixel_y < max_y; pixel_y++) {
        vec3 bary_coord_x = bary_coord_y;
        for (int pixel_x = min_x; pixel_x < max_x; pixel_x++) {
            if (is_inside_ss(bary_coord_x)) {
                // screen space interpolate
                auto w_ss = std::make_tuple(bary_coord_x.x(), bary_coord_x.y(),
                                            bary_coord_x.z());

                float z = interpolate_z(w_ss);
                if (0.f < z && z < z_buffer->at(pixel_x, pixel_y)) {
                    z_buffer->at(pixel_x, pixel_y) = z;

                    // view space interpolate
                    auto w = interpolate_weights(w_ss, z);
                    vec3 pos = interpolate(
                        std::make_tuple(v1->pos, v2->pos, v3->pos), w);

                    vec3 normal;
                    if (normals.empty()) {
                        normal = this->normal();
                    } else {
                        normal = interpolate(
                            std::make_tuple(normals[0], normals[1], normals[2]),
                            w);
                    }

                    vec3 shading =
                        fragment_shader->shade(pos, normal, material);

                    // write into frame buffer
                    frame_buffer->at(pixel_x, pixel_y, 0) =
                        truncate_color(shading.x());
                    frame_buffer->at(pixel_x, pixel_y, 1) =
                        truncate_color(shading.y());
                    frame_buffer->at(pixel_x, pixel_y, 2) =
                        truncate_color(shading.z());
                    // frame_buffer->at(pixel_x, pixel_y, 0) =
                    // truncate_color(z); frame_buffer->at(pixel_x, pixel_y, 1)
                    // = truncate_color(z); frame_buffer->at(pixel_x, pixel_y,
                    // 2) = truncate_color(z);
                }
            }
            bary_coord_x += bary_coord_dx;
        }
        bary_coord_y += bary_coord_dy;
    }
}