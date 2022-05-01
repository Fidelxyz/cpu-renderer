#include "geometry/triangle.hpp"

#include <algorithm>
#include <cmath>
#include <opencv2/opencv.hpp>

#include "global.hpp"

Triangle::Triangle() {}

float Triangle::cross2d(const vec2 &v1, const vec2 &v2) {
    return v1.x() * v2.y() - v2.x() - v1.y();
}

float Triangle::truncate_x_ss(float x, Camera *camera) {
    x = std::min(x, static_cast<float>(camera->width));
    x = std::max(x, 0.f);
    return x;
}

float Triangle::truncate_y_ss(float y, Camera *camera) {
    y = std::min(y, static_cast<float>(camera->height));
    y = std::max(y, 0.f);
    return y;
}

bool Triangle::is_inside_ss(const vec2 &pos) const {
    auto v1 = vec2(vertices[0]->screen_pos.x(), vertices[0]->screen_pos.y());
    auto v2 = vec2(vertices[1]->screen_pos.x(), vertices[1]->screen_pos.y());
    auto v3 = vec2(vertices[2]->screen_pos.x(), vertices[2]->screen_pos.y());

    float c1 = cross2d(pos - v1, v2 - v1);
    float c2 = cross2d(pos - v2, v3 - v2);
    float c3 = cross2d(pos - v3, v1 - v3);

    return (c1 > -EPS && c2 > -EPS && c3 > -EPS) || (c1 < EPS && c2 < EPS && c3 < EPS);
}

void Triangle::rasterize(cv::Mat *frame_buffer, cv::Mat *z_buffer,
                         FragmentShader *fragment_shader,
                         Camera *camera) const {
    int min_x =
        truncate_x_ss(std::floor(std::min({vertices[0]->screen_pos.x(),
                                           vertices[1]->screen_pos.x(),
                                           vertices[2]->screen_pos.x()})),
                      camera);
    int min_y =
        truncate_y_ss(std::floor(std::min({vertices[0]->screen_pos.y(),
                                           vertices[1]->screen_pos.y(),
                                           vertices[2]->screen_pos.y()})),
                      camera);
    int max_x =
        truncate_x_ss(std::ceil(std::max({vertices[0]->screen_pos.x(),
                                          vertices[1]->screen_pos.x(),
                                          vertices[2]->screen_pos.x()})),
                      camera);
    int max_y =
        truncate_y_ss(std::ceil(std::max({vertices[0]->screen_pos.y(),
                                          vertices[1]->screen_pos.y(),
                                          vertices[2]->screen_pos.y()})),
                      camera);

    for (int pixel_x = min_x; pixel_x < max_x; pixel_x++) {
        for (int pixel_y = min_y; pixel_y < max_y; pixel_y++) {
            float x = pixel_x + 0.5f;
            float y = pixel_y + 0.5f;
            if (is_inside_ss(vec2(x, y))) {
                frame_buffer->at<cv::Vec3f>(y, x) = cv::Vec3f(1, 1, 1);
            }
        }
    }
}