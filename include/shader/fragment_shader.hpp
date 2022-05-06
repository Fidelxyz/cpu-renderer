#ifndef FRAGMENT_SHADER_H
#define FRAGMENT_SHADER_H

#include <vector>

#include "camera.hpp"
#include "geometry/material.hpp"
#include "global.hpp"
#include "light.hpp"

class FragmentShader {
   public:
    std::vector<Light *> lights;
    vec3 eye_pos;

    FragmentShader(Camera *camera, std::vector<Light *> lights);
    vec3 shade(const vec3 &pos, const vec3 &normal, Material *material);
};

#endif