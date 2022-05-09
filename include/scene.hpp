#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include "camera.hpp"
#include "geometry/material.hpp"
#include "geometry/object.hpp"
#include "light.hpp"

class Scene {
   public:
    std::vector<Object> objects;
    std::vector<Light> lights;
    std::vector<Material> materials;

    Camera camera;
    Scene();
};

#endif