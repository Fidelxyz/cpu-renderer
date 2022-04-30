#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include "camera.hpp"
#include "geometry/object.hpp"
#include "light.hpp"

class Scene {
   public:
    std::vector<Object *> objects;
    std::vector<Light *> lights;

    Camera *camera;
};

#endif