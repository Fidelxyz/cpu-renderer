#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <tuple>
#include <vector>

#include "camera.hpp"
#include "geometry/material.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"
#include "shader/fragment_shader.hpp"
#include "utils/buffer.hpp"

class Triangle {
   public:
    std::vector<Vertex *> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texcoords;
    Material *material = nullptr;

    Triangle();

    // Return if the point is inside the triangle in the screen space.
    bool is_inside_ss(const vec3 &w_ss) const;

    void rasterize(Buffer<float> *frame_buffer, Buffer<float> *z_buffer,
                   FragmentShader *fragment_shader, const Camera &camera) const;

    vec3 normal() const;

   private:
    static float cross2d(const vec2 &v1, const vec2 &v2);
    static float truncate_x_ss(float x, const Camera &camera);
    static float truncate_y_ss(float y, const Camera &camera);
    static float truncate_color(float color);

    template <typename T>
    static T interpolate(const std::tuple<T, T, T> &vals,
                         const std::tuple<float, float, float> &weights);

    // Return the interpolate weights in screen spaces
    vec3 interpolate_weights_ss(const vec2 &screen_pos) const;

    float interpolate_z(const std::tuple<float, float, float> &weights) const;

    // Return the interpolate weights in view spaces
    std::tuple<float, float, float> interpolate_weights(
        const std::tuple<float, float, float> &weights_ss, const float z) const;
};

#endif