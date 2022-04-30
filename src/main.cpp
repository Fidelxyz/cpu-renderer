#include <cstdio>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "camera.hpp"
#include "geometry/object.hpp"
#include "global.hpp"
#include "light.hpp"
#include "scene.hpp"
#include "shader/fragment_shader.hpp"
#include "shader/vertex_shader.hpp"

void render(Scene& scene) {
    auto vertex_shader = VertexShader(scene.camera);
    auto fragment_shader = FragmentShader(scene.lights);

    for (auto object : scene.objects) {
        for (auto vertex : object->vertices) {
            vertex_shader.shade(&vertex);
        }
    }
}

int main(int argc, char* argv[]) {
    auto obj = Object();
    obj.load_model("../res/bunny/bunny.obj", "../res/bunny");

    auto scene = Scene();
    scene.objects.push_back(&obj);

    auto light = Light(vec3(1, 1, 1));
    scene.lights.push_back(&light);

    auto camera = Camera(
        camera::POS, camera::UP_DIR, camera::LOOK_DIR, camera::FOV,
        static_cast<float>(screen::width) / static_cast<float>(screen::height),
        camera::NEAR_PLANE, camera::FAR_PLANE);
    scene.camera = &camera;

    return 0;
}