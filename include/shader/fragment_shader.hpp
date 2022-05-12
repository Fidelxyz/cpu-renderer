#ifndef FRAGMENT_SHADER_H
#define FRAGMENT_SHADER_H

#include <vector>

#include "camera.hpp"
#include "geometry/material.hpp"
#include "global.hpp"
#include "light.hpp"
#include "texture.hpp"

class FragmentShader {
   public:
    std::vector<Light> *lights;
    vec3 eye_pos;

    FragmentShader(const Camera &camera, std::vector<Light> &lights);
    vec3 shade(const vec3 &pos, const vec3 &normal, const vec2 &uv,
               Material *material);
};

#endif