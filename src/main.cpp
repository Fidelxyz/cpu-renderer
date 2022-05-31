#include <array>
#include <chrono>
#include <memory>

#include "config.hpp"
#include "effects/msaa.hpp"
#include "geometry/object.hpp"
#include "global.hpp"
#include "scene/camera.hpp"
#include "scene/light.hpp"
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
            object.model_transform();
            for (auto &vertex : object.vertices) {
                vertex_shader.shade(vertex.get());
            }
        }
    }

    Texture<std::array<float, msaa::MSAA_LEVEL>> z_buffer(
        scene.camera.width, scene.camera.height, msaa::texture_init_val(1.f));

    Texture<std::array<vec3, msaa::MSAA_LEVEL>> frame_buffer(
        scene.camera.width, scene.camera.height,
        msaa::texture_init_val(vec3(0, 0, 0)));

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

    Texture<vec3> frame_result = msaa::msaa_filter(frame_buffer);
    frame_result.write_img("out.png", false);
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