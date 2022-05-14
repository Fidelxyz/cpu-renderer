#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include "geometry/object.hpp"
#include "scene/camera.hpp"
#include "scene/light.hpp"

class Scene {
   public:
    std::vector<Object> objects;
    std::vector<Light> lights;

    Camera camera;
};

#endif