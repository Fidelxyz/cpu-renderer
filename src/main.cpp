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

    size_t buffer_depth = scene.camera.msaa ? 4 : 1;
    Texture<float> z_buffer(scene.camera.width, scene.camera.height,
                            buffer_depth, 1.f);
    Texture<vec3> frame_buffer(scene.camera.width, scene.camera.height,
                               buffer_depth, vec3(0, 0, 0));

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

    frame_buffer.write_img("out.png", false);
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

// TODO MSAA
// TODO Multi-thread