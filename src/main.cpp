#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
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
#include "texture.hpp"
#include "utils/timer.hpp"

void render(Scene &scene) {
    auto vertex_shader = VertexShader(scene.camera);
    auto fragment_shader = FragmentShader(scene.camera, scene.lights);

    {
        Timer timer("Vertex shader");
        for (auto &object : scene.objects) {
            object.model_transform();
            for (auto &vertex : object.vertices) {
                vertex_shader.shade(vertex.get());
            }
        }
    }

    Texture<float> z_buffer(scene.camera.width, scene.camera.height, 1.f);
    Texture<vec3> frame_buffer(scene.camera.width, scene.camera.height,
                               vec3(0, 0, 0));

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

    frame_buffer.write_img("out.png");
}

int main(int argc, char *argv[]) {
    auto scene = Scene();
    {
        Timer timer("Load");
        auto config = Config("../example/config.yaml");
        if (!config.load_scene(&scene)) return 0;
    }

    {  // render
        Timer timer("Frame render");
        render(scene);
    }

    return 0;
}

// TODO Material 中的 Texture 指针（Texture 复用）
// TODO Gamma correction