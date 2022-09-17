#include "geometry/triangle.hpp"

#include <omp.h>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <cmath>

#include "global.hpp"
#include "texture/mipmap.hpp"

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

bool Triangle::is_culled_normal(const Camera &camera,
                                CullMethod cull_method) const {
    if (normals.empty()) {
        vec3 normal = this->normal();
        switch (cull_method) {
            case CULL_BACK: {
                for (size_t i = 0; i < 3; i++) {
                    if (normal.dot(
                            (camera.pos - vertices[i]->pos).normalized()) >
                        -EPS) {
                        return false;
                    }
                }
                return true;
            }
            case CULL_FRONT: {
                for (size_t i = 0; i < 3; i++) {
                    if (normal.dot(
                            (camera.pos - vertices[i]->pos).normalized()) <
                        EPS) {
                        return false;
                    }
                }
                return true;
            }
            case NO_CULL:
                return false;
        }
    } else {
        switch (cull_method) {
            case CULL_BACK: {
                for (size_t i = 0; i < 3; i++) {
                    if (normals[i]->dot(
                            (camera.pos - vertices[i]->pos).normalized()) >
                        -EPS) {
                        return false;
                    }
                }
                return true;
            }
            case CULL_FRONT: {
                for (size_t i = 0; i < 3; i++) {
                    if (normals[i]->dot(
                            (camera.pos - vertices[i]->pos).normalized()) <
                        EPS) {
                        return false;
                    }
                }
                return true;
            }
            case NO_CULL:
                return false;
        }
    }
    return false;
}

bool Triangle::is_culled_normal(const vec3 &normal, const vec3 &pos,
                                const Camera &camera,
                                CullMethod cull_method) const {
    switch (cull_method) {
        case CULL_BACK:
            return normal.dot((camera.pos - pos).normalized()) < -EPS;
        case CULL_FRONT:
            return normal.dot((camera.pos - pos).normalized()) > EPS;
        case NO_CULL:
            return false;
    }
    return false;
}

bool Triangle::is_culled_view(const Camera &camera) const {
    // cull boundary
    const float L1[] = {-EPS, -EPS, -EPS};
    const float R1[] = {camera.width + EPS, camera.height + EPS, 1.f + EPS};

    // for each dimension
    for (size_t i = 0; i < 3; i++) {
        // culled if all vertices are outside the view (in the same side)

        // test < 0
        if (vertices[0]->screen_pos[i] < L1[i] &&
            vertices[1]->screen_pos[i] < L1[i] &&
            vertices[2]->screen_pos[i] < L1[i]) {
            return true;
        }

        // test > 1
        if (vertices[0]->screen_pos[i] > R1[i] &&
            vertices[1]->screen_pos[i] > R1[i] &&
            vertices[2]->screen_pos[i] > R1[i]) {
            return true;
        }

        if (vertices[0]->w < EPS || vertices[1]->w < EPS ||
            vertices[2]->w < EPS) {
            return true;
        }
    }
    return false;
}

bool Triangle::is_inside_ss(const vec3 &w) const {
    return -EPS < w.x() && -EPS < w.y() && -EPS < w.z();
}

vec3 Triangle::normal() const {
    return (vertices[0]->pos - vertices[1]->pos)
        .cross(vertices[0]->pos - vertices[2]->pos)
        .normalized();
}

vec3 Triangle::barycoord_ss(const vec2 &screen_pos) const {
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

float Triangle::interpolate_z_ss(const vec3 &barycoord_ss) const {
    float w1 = barycoord_ss.x();
    float w2 = barycoord_ss.y();
    float w3 = barycoord_ss.z();
    float z1 = vertices[0]->screen_pos.z();
    float z2 = vertices[1]->screen_pos.z();
    float z3 = vertices[2]->screen_pos.z();
    return w1 * z1 + w2 * z2 + w3 * z3;
}

std::tuple<float, float, float> Triangle::corrected_barycoord(
    const vec3 &barycoord_ss) const {
    float w1 = vertices[0]->w;
    float w2 = vertices[1]->w;
    float w3 = vertices[2]->w;

    float alpha = barycoord_ss.x();
    float beta = barycoord_ss.y();
    float gamma = barycoord_ss.z();
    float l = alpha / w1 + beta / w2 + gamma / w3;

    return std::make_tuple(alpha / w1 / l, beta / w2 / l, gamma / w3 / l);
}

std::tuple<vec2, vec2> Triangle::calc_uv(
    const std::tuple<float, float, float> &w_shading,
    const vec3 &barycoord_shading,
    const std::tuple<vec3, vec3> &barycoord_lod_sample_delta) const {
    vec2 uv, duv;
    if (texcoords.empty()) {
        uv = vec2(0, 0);
        duv = vec2(1, 1);
    } else {
        auto [barycoord_lod_sample_x_delta, barycoord_lod_sample_y_delta] =
            barycoord_lod_sample_delta;

        auto texcoord_tuple =
            std::make_tuple(*texcoords[0], *texcoords[1], *texcoords[2]);

        // perspective-corrected interpolate
        auto w_x = corrected_barycoord(barycoord_shading +
                                       barycoord_lod_sample_x_delta);
        auto w_y = corrected_barycoord(barycoord_shading +
                                       barycoord_lod_sample_y_delta);

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