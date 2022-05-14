#include <array>
#include <chrono>
#include <memory>

#include "config.hpp"
#include "geometry/object.hpp"
#include "global.hpp"
#include "scene/camera.hpp"
#include "scene/light.hpp"
#include "scene/scene.hpp"
#include "shader/fragment_shader.hpp"
#include "shader/vertex_shader.hpp"
#include "texture.hpp"
#include "utils/timer.hpp"

#ifdef MSAA
#include "effects/msaa.hpp"
#endif

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

#ifdef MSAA
    Texture<std::array<float, 4>> z_buffer(
        scene.camera.width, scene.camera.height,
        std::array<float, 4>{1.f, 1.f, 1.f, 1.f});

    Texture<std::array<vec3, 4>> frame_buffer(
        scene.camera.width, scene.camera.height,
        std::array<vec3, 4>{vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0),
                            vec3(0, 0, 0)});
#else
    Texture<float> z_buffer(scene.camera.width, scene.camera.height, 1.f);
    Texture<vec3> frame_buffer(scene.camera.width, scene.camera.height,
                               vec3(0, 0, 0));
#endif

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

#ifdef MSAA
    Texture<vec3> frame_result = msaa::msaa_filter(frame_buffer);
    frame_result.write_img("out.png", false);
#else
    frame_buffer.write_img("out.png", false);
#endif
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

// TODO Multi-thread
// TODO MIPMAP