#pragma once
#ifndef FRAGMENT_SHADER_H
#define FRAGMENT_SHADER_H

#include <vector>

#include "global.hpp"
#include "scene/camera.hpp"
#include "scene/light.hpp"
#include "scene/material.hpp"

class FragmentShader {
   public:
    std::vector<Light> *lights;
    vec3 eye_pos;

    FragmentShader(const Camera &camera, std::vector<Light> &lights);
    vec3 shade(const vec3 &pos, const vec3 &normal, const vec2 &uv,
               const vec2 &duv, Material *material);
};

#endif