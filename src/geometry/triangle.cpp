#include "geometry/triangle.hpp"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <cmath>

#include "global.hpp"
#include "texture/mipmap.hpp"

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
                     const vec2 &uv, const vec2 &duv,
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

    return fragment_shader->shade(pos, normal, uv, duv, material.get());
}

std::tuple<vec2, vec2> Triangle::calc_uv(
    const vec3 &bary_coord_shading, const vec3 &bary_coord_lod_x_delta,
    const vec3 &bary_coord_lod_y_delta) const {
    vec2 uv, duv;
    if (texcoords.empty()) {
        uv = vec2(0, 0);
        duv = vec2(1, 1);
    } else {
        auto texcoord_tuple =
            std::make_tuple(*texcoords[0], *texcoords[1], *texcoords[2]);

        // perspective-corrected interpolate
        auto w_shading = corrected_bary_coord(bary_coord_shading);
        auto w_x =
            corrected_bary_coord(bary_coord_shading + bary_coord_lod_x_delta);
        auto w_y =
            corrected_bary_coord(bary_coord_shading + bary_coord_lod_y_delta);

        uv = interpolate(texcoord_tuple, w_shading);
        vec2 ddx =
            (interpolate(texcoord_tuple, w_x) - uv) / mipmap::LOD_SAMPLE_DELTA;
        vec2 ddy =
            (interpolate(texcoord_tuple, w_y) - uv) / mipmap::LOD_SAMPLE_DELTA;
        float du = (std::fabs(ddx.x()) + std::fabs(ddy.x())) / 2.f;
        float dv = (std::fabs(ddx.y()) + std::fabs(ddy.y())) / 2.f;
        duv = vec2(du, dv);
    }
    return std::make_tuple(uv, duv);
}

void Triangle::rasterize(frame_buffer_t *frame_buffer, z_buffer_t *z_buffer,
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

    vec3 bary_coord_samples_delta[msaa::MSAA_LEVEL];
    for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
        bary_coord_samples_delta[i] =
            bary_coord_ss(vec2(min_x, min_y) + msaa::samples_coord_delta[i]) -
            bary_coord_init;
    }

    vec3 bary_coord_lod_x_delta =
        bary_coord_ss(
            vec2(min_x + 0.5f + mipmap::LOD_SAMPLE_DELTA, min_y + 0.5f)) -
        bary_coord_init;
    vec3 bary_coord_lod_y_delta =
        bary_coord_ss(
            vec2(min_x + 0.5f, min_y + 0.5f + mipmap::LOD_SAMPLE_DELTA)) -
        bary_coord_init;

    vec3 bary_coord_y = bary_coord_init;
    for (int pixel_y = min_y; pixel_y < max_y; pixel_y++) {
        vec3 bary_coord_x = bary_coord_y;
        for (int pixel_x = min_x; pixel_x < max_x; pixel_x++) {
            vec3 bary_coord_samples[msaa::MSAA_LEVEL];
            unsigned char covered_flag = 0;
            for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                bary_coord_samples[i] =
                    bary_coord_x + bary_coord_samples_delta[i];
                if (is_inside_ss(bary_coord_samples[i])) {
                    float z = interpolate_z_ss(bary_coord_samples[i]);
                    if (0.f < z && z < z_buffer->at(pixel_x, pixel_y)[i]) {
                        z_buffer->at(pixel_x, pixel_y)[i] = z;
                        covered_flag |= 1u << i;
                    }
                }
            }

            if (covered_flag) {
                const bool full_covered =
                    covered_flag == (1u << msaa::MSAA_LEVEL) - 1;

                vec3 bary_coord_shading;

                if (full_covered) {  // All samples covered
                    bary_coord_shading = bary_coord_x;
                } else {  // Partical samples covered
                    bary_coord_shading = vec3(0, 0, 0);
                    int covered_count = 0;
                    for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                        if ((covered_flag >> i) & 1) {
                            bary_coord_shading += bary_coord_samples[i];
                            covered_count++;
                        }
                    }
                    bary_coord_shading /= covered_count;
                }

                auto [uv, duv] =
                    calc_uv(bary_coord_shading, bary_coord_lod_x_delta,
                            bary_coord_lod_y_delta);
                vec3 shading = shade(pixel_x, pixel_y, bary_coord_shading, uv,
                                     duv, fragment_shader);

                if (full_covered) {
                    for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                        frame_buffer->at(pixel_x, pixel_y)[i] = shading;
                    }
                } else {
                    for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                        if (covered_flag & (1u << i)) {
                            frame_buffer->at(pixel_x, pixel_y)[i] = shading;
                        }
                    }
                }
            }

            bary_coord_x += bary_coord_dx;
        }
        bary_coord_y += bary_coord_dy;
    }
}
