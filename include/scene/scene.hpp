#pragma once
#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include "geometry/object.hpp"
#include "scene/camera.hpp"
#include "light/light.hpp"

class Scene {
   public:
    std::vector<Object> objects;
    std::vector<Light> lights;

    Camera camera;

    bool enable_pbr = false;
};

#endif