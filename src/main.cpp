#include <chrono>
#include <cstdio>
#include <memory>
#include <opencv2/opencv.hpp>
#include <unordered_map>

#include "camera.hpp"
#include "config.hpp"
#include "geometry/material.hpp"
#include "geometry/object.hpp"
#include "global.hpp"
#include "light.hpp"
#include "scene.hpp"
#include "shader/fragment_shader.hpp"
#include "shader/vertex_shader.hpp"
#include "utils/buffer.hpp"
#include "utils/timer.hpp"

void render(Scene &scene) {
    auto vertex_shader = VertexShader(scene.camera);
    auto fragment_shader = FragmentShader(scene.camera, scene.lights);

    {
        Timer timer("Vertex shader");
        for (auto &object : scene.objects) {
            for (auto &vertex : object.vertices) {
                vertex_shader.shade(&vertex, object);
            }
        }
    }

    Buffer<float> z_buffer(scene.camera.width, scene.camera.height, 1, 1.f);
    Buffer<float> frame_buffer(scene.camera.width, scene.camera.height, 3, 0.f);

    {
        Timer timer("Trianglar rasterize");
        for (auto &object : scene.objects) {
            for (auto &shape : object.shapes) {
                for (auto &triangle : shape.triangles) {
                    triangle.rasterize(&frame_buffer, &z_buffer,
                                       &fragment_shader, scene.camera);
                }
            }
        }
    }

    auto image = cv::Mat(scene.camera.height, scene.camera.width, CV_8UC3);
    for (int y = 0; y < scene.camera.height; y++) {
        for (int x = 0; x < scene.camera.width; x++) {
            image.at<cv::Vec3b>(y, x)[2] = frame_buffer.at(x, y, 0) * 255.f;
            image.at<cv::Vec3b>(y, x)[1] = frame_buffer.at(x, y, 1) * 255.f;
            image.at<cv::Vec3b>(y, x)[0] = frame_buffer.at(x, y, 2) * 255.f;
        }
    }
    cv::imwrite("out.png", image);
}

int main(int argc, char *argv[]) {
    auto config = Config("../example/config.yaml");
    auto scene = Scene();
    if (!config.load_scene(&scene)) return 0;

    {  // render
        Timer timer("Frame render");
        render(scene);
    }

    return 0;
}