#include <chrono>
#include <cstdio>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "buffer.hpp"
#include "camera.hpp"
#include "geometry/material.hpp"
#include "geometry/object.hpp"
#include "global.hpp"
#include "light.hpp"
#include "scene.hpp"
#include "shader/fragment_shader.hpp"
#include "shader/vertex_shader.hpp"

void render(Scene &scene) {
    auto vertex_shader = VertexShader(scene.camera);
    auto fragment_shader = FragmentShader(scene.camera, scene.lights);

    for (auto &object : scene.objects) {
        for (auto &vertex : object->vertices) {
            vertex_shader.shade(&vertex);
        }
    }

    Buffer z_buffer(scene.camera->width, scene.camera->height, 1, 1.f);
    Buffer frame_buffer(scene.camera->width, scene.camera->height, 3);

    for (auto &object : scene.objects) {
        for (auto &shape : object->shapes) {
            for (auto &triangle : shape.triangles) {
                triangle.rasterize(&frame_buffer, &z_buffer, &fragment_shader,
                                   scene.camera);
            }
        }
    }

    auto image = cv::Mat(scene.camera->height, scene.camera->width, CV_8UC3);
    for (int y = 0; y < scene.camera->height; y++) {
        for (int x = 0; x < scene.camera->width; x++) {
            image.at<cv::Vec3b>(y, x)[0] = frame_buffer.at(x, y, 0) * 255;
            image.at<cv::Vec3b>(y, x)[1] = frame_buffer.at(x, y, 1) * 255;
            image.at<cv::Vec3b>(y, x)[2] = frame_buffer.at(x, y, 2) * 255;
        }
    }
    auto display_image =
        cv::Mat(scene.camera->height, scene.camera->width, CV_8UC3);
    cv::cvtColor(image, display_image, cv::COLOR_RGB2BGR);
    cv::imwrite("out.png", display_image);
}

int main(int argc, char *argv[]) {
    auto obj = Object();
    if (!obj.load_model("../res/bunny/bunny.obj", "../res/bunny")) return 0;

    auto scene = Scene();
    scene.objects.push_back(&obj);

    auto light = Light(vec3(0.5, 1, -2), vec3(1, 1, 0.8), 10);
    scene.lights.push_back(&light);

    auto camera = Camera(camera::POS, camera::LOOK_DIR, camera::UP_DIR,
                         camera::FOV, camera::NEAR_PLANE, camera::FAR_PLANE,
                         camera::WIDTH, camera::HEIGHT);
    scene.camera = &camera;

    auto material = Material();
    material.ambient = vec3(0.05, 0.05, 0.05);
    material.specular = vec3(0.5, 0.5, 0.5);
    material.diffuse = vec3(0.1, 0.1, 0.1);
    material.shininess = 20;

    for (auto &object : scene.objects) {
        for (auto &shape : object->shapes) {
            for (auto &triangle : shape.triangles) {
                triangle.material = &material;
            }
        }
    }

    // render

    auto start_time = std::chrono::high_resolution_clock::now();

    render(scene);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    printf("Frame render time: %ldus\n", duration.count());

    return 0;
}