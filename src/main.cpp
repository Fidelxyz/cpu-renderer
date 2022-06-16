#include <omp.h>

#include <array>
#include <chrono>
#include <functional>
#include <memory>

#include "config.hpp"
#include "effects/msaa.hpp"
#include "effects/outline.hpp"
#include "effects/rimlight.hpp"
#include "geometry/object.hpp"
#include "global.hpp"
#include "light/light.hpp"
#include "scene/camera.hpp"
#include "scene/scene.hpp"
#include "shader/fragment_shader.hpp"
#include "shader/vertex_shader.hpp"
#include "texture/texture.hpp"
#include "utils/timer.hpp"

void render(Scene &scene) {
    auto vertex_shader = VertexShader(scene.camera);
    auto fragment_shader = FragmentShader(scene.camera, scene.lights);

    {
        Timer timer("Vertex shader");
        for (auto &object : scene.objects) {
            object.do_model_transform();
#pragma omp parallel for
            for (auto &vertex : object.vertices) {
                vertex_shader.shade(vertex.get());
            }
        }
    }

    Texture<std::array<float, msaa::MSAA_LEVEL>> z_buffer(
        scene.camera.width, scene.camera.height, msaa::texture_init_val(1.f));

    Texture<std::array<vec3, msaa::MSAA_LEVEL>> frame_buffer(
        scene.camera.width, scene.camera.height,
        msaa::texture_init_val(vec3(0.5, 0.5, 0.5)));

    // init mutex
    Texture<omp_lock_t> mutex(scene.camera.width, scene.camera.height);
    for (auto &m : mutex) {
        omp_init_lock(&m);
    }

    {
        Timer timer("Trianglar rasterize");
        for (auto &object : scene.objects) {
            for (auto &shape : object.shapes) {
#pragma omp parallel for
                for (auto &triangle : shape.triangles) {
                    triangle.rasterize(&mutex, &frame_buffer, &z_buffer,
                                       &fragment_shader, scene.camera,
                                       Triangle::CULL_BACK);
                }
            }
        }
    }

    {
        Timer timer("Outline pass");
        auto outline_vertex_shader = outline::OutlineVertexShader(scene.camera);
        auto outline_fragment_shader =
            outline::OutlineFragmentShader(scene.camera);

        for (auto &object : scene.objects) {
#pragma omp parallel for
            for (auto &vertex : object.vertices) {
                outline_vertex_shader.shade(vertex.get());
                vertex_shader.shade(vertex.get());
            }

            for (auto &shape : object.shapes) {
#pragma omp parallel for
                for (auto &triangle : shape.triangles) {
                    triangle.rasterize(&mutex, &frame_buffer, &z_buffer,
                                       &outline_fragment_shader, scene.camera,
                                       Triangle::CULL_FRONT);
                }
            }
        }
    }

    {
        Timer timer("Rimlight");
        rimlight::rimlight(&frame_buffer, z_buffer, scene.camera);
    }

    Texture<vec3> frame_result;
    // Texture<float> frame_result;
    {
        Timer timer("MSAA Filter");
        frame_result = msaa::msaa_filter(frame_buffer);
        // frame_result = msaa::msaa_filter(z_buffer);
    }

    frame_result.write_img("out.png", false);
    // frame_result.write_img("out.png", true);

    // destroy mutex
    for (auto &m : mutex) {
        omp_destroy_lock(&m);
    }
}

int main(int argc, char *argv[]) {
    auto scene = Scene();
    {
        Timer timer("Load");
        auto config = Config("../example/config.yaml");

        // load threads_num
        int threads_num;
        if (config.load_threads_num(&threads_num)) {
            omp_set_num_threads(threads_num);
        }

        // load scene
        if (!config.load_scene(&scene)) return 0;
    }

    {  // render
        Timer timer("Frame render");
        render(scene);
    }

    return 0;
}