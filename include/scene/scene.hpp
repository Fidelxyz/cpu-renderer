#pragma once
#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include "geometry/object.hpp"
#include "light/light.hpp"
#include "scene/camera.hpp"

class Scene {
   public:
    std::vector<Object> objects;
    std::vector<Light> lights;

    Camera camera;

    vec3 background_color = vec3(0, 0, 0);
    bool enable_rimlight = false;
};

#endif