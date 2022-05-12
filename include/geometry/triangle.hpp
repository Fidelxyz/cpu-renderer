#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <memory>
#include <tuple>
#include <vector>

#include "camera.hpp"
#include "geometry/material.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"
#include "shader/fragment_shader.hpp"
#include "texture.hpp"

class Triangle {
   public:
    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<std::shared_ptr<vec3>> normals;
    std::vector<std::shared_ptr<vec2>> texcoords;
    std::shared_ptr<Material> material;

    Triangle();

    vec3 normal() const;
    void normal_transform(const mat3 normal_transform);

    // Return if the point is inside the triangle in the screen space.
    bool is_inside_ss(const vec3 &w_ss) const;

    void rasterize(Texture<vec3> *frame_buffer, Texture<float> *z_buffer,
                   FragmentShader *fragment_shader, const Camera &camera) const;

   private:
    static float cross2d(const vec2 &v1, const vec2 &v2);
    static float truncate_x_ss(float x, const Camera &camera);
    static float truncate_y_ss(float y, const Camera &camera);
    static float truncate_color(float color);
    static vec3 truncate_color(const vec3 &color);

    vec3 bary_coord_ss(const vec2 &screen_pos) const;

    float interpolate_z_ss(const vec3 &bary_coord_ss) const;

    std::tuple<float, float, float> corrected_bary_coord(
        const vec3 &bary_coord_ss) const;

    template <typename T>
    static T interpolate(const std::tuple<T, T, T> &vals,
                         const std::tuple<float, float, float> &weights);

    vec3 shade(size_t pixel_x, size_t pixel_y, const vec3 &bary_coord,
               FragmentShader *fragment_shader) const;
};

#endif