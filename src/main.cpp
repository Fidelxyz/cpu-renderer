#include <omp.h>

#include <array>
#include <chrono>
#include <functional>
#include <memory>

#include "config.hpp"
#include "effects/bloom.hpp"
#include "effects/msaa.hpp"
#include "effects/outline.hpp"
#include "effects/rimlight.hpp"
#include "effects/ssao.hpp"
#include "geometry/object.hpp"
#include "global.hpp"
#include "light/light.hpp"
#include "scene/camera.hpp"
#include "scene/scene.hpp"
#include "shader/fragment_shader.hpp"
#include "shader/vertex_shader.hpp"
#include "texture/buffer.hpp"
#include "texture/texture.hpp"
#include "utils/progress_bar.hpp"
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

    Buffer buffer;

    {
        Timer timer("Initialize buffer");

        // init mutex
        buffer.mutex = std::make_shared<Texture<omp_lock_t>>(
            scene.camera.width, scene.camera.height);
        for (auto &m : *buffer.mutex) {
            omp_init_lock(&m);
        }

        buffer.z_buffer = std::make_shared<z_buffer_t>(
            scene.camera.width, scene.camera.height,
            msaa::texture_init_val(1.f));

        buffer.frame_buffer = std::make_shared<frame_buffer_t>(
            scene.camera.width, scene.camera.height,
            msaa::texture_init_val(scene.background_color));

        buffer.pos_buffer = std::make_shared<msaa_texture_t<vec3>>(
            scene.camera.width, scene.camera.height,
            msaa::texture_init_val(vec3(0, 0, 0)));

        buffer.normal_buffer = std::make_shared<msaa_texture_t<vec3>>(
            scene.camera.width, scene.camera.height,
            msaa::texture_init_val(vec3(0, 0, 0)));

        buffer.full_covered = std::make_shared<Texture<bool>>(
            scene.camera.width, scene.camera.height, true);
    }

    {
        Timer timer("Trianglar rasterization");
        for (auto &object : scene.objects) {
            ProgressBar progress("Rendering shapes", object.shapes.size());
            for (auto &shape : object.shapes) {
#pragma omp parallel for
                for (auto &triangle : shape.triangles) {
                    triangle.rasterize(&buffer, &fragment_shader, scene.camera,
                                       Triangle::CULL_BACK);
                }
                progress.update();
            }
        }
    }

    {
        Timer timer("Outline pass");
        auto outline_vertex_shader = outline::OutlineVertexShader(scene.camera);
        auto outline_fragment_shader =
            outline::OutlineFragmentShader(scene.camera);

        for (auto &object : scene.objects) {
            if (object.shading_type != "cel") continue;
#pragma omp parallel for
            for (auto &vertex : object.vertices) {
                outline_vertex_shader.shade(vertex.get());
                vertex_shader.shade(vertex.get());
            }

            for (auto &shape : object.shapes) {
#pragma omp parallel for
                for (auto &triangle : shape.triangles) {
                    triangle.rasterize(&buffer, &outline_fragment_shader,
                                       scene.camera, Triangle::CULL_FRONT);
                }
            }
        }
    }

    if (scene.enable_rimlight) {
        Timer timer("Rimlight");
        rimlight::rimlight(&buffer, scene.camera);
    }

    Texture<vec3> frame_result;
    {
        Timer timer("MSAA filter");
        frame_result =
            msaa::msaa_filter(*buffer.full_covered, *buffer.frame_buffer);
    }

    {
        Timer timer("SSAO filter");
        Texture<float> z_buffer_result =
            msaa::msaa_filter(*buffer.full_covered, *buffer.z_buffer);
        frame_result = ssao::ssao_filter(&buffer, frame_result, z_buffer_result,
                                         &vertex_shader);
    }

    if (scene.enable_bloom) {
        Timer timer("Bloom filter");
        frame_result =
            bloom::bloom_filter(frame_result, scene.bloom_strength,
                                scene.bloom_radius, scene.bloom_iteration);
    }

    {
        Timer timer("Save image");
        frame_result.write_img("out.png", false);
    }

    // destroy mutex
    for (auto &m : *buffer.mutex) {
        omp_destroy_lock(&m);
    }
}

int main(int argc, char *argv[]) {
    auto scene = Scene();
    {
        Timer timer("Load");
        std::string config_path;
        if (argc == 1) {
            config_path = "../example/default-config.yaml";
        } else {
            config_path = argv[1];
        }
        auto config = Config(config_path);

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