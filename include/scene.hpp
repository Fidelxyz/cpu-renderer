#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include "camera.hpp"
#include "geometry/material.hpp"
#include "geometry/object.hpp"
#include "light.hpp"
#include "texture.hpp"

class Scene {
   public:
    std::vector<Object> objects;
    std::vector<Light> lights;

    Camera camera;
    Scene();
};

#endif