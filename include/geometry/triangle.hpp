#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <opencv2/opencv.hpp>
#include <vector>

#include "camera.hpp"
#include "geometry/material.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"
#include "shader/fragment_shader.hpp"

class Triangle {
   public:
    std::vector<Vertex *> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texcoords;
    Material *material = nullptr;

    Triangle();

    // Return if the point is inside the triangle in the screen space.
    bool is_inside_ss(const vec2 &pos) const;

    void rasterize(cv::Mat *frame_buffer, cv::Mat *z_buffer,
                   FragmentShader *fragment_shader, Camera *camera) const;

   private:
    static float cross2d(const vec2 &v1, const vec2 &v2);
    static float truncate_x_ss(float x, Camera *camera);
    static float truncate_y_ss(float y, Camera *camera);
};

#endif