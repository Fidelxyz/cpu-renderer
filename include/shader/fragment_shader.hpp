#pragma once
#ifndef FRAGMENT_SHADER_H
#define FRAGMENT_SHADER_H

#include <functional>
#include <vector>

#include "global.hpp"
#include "light/light.hpp"
#include "scene/camera.hpp"
#include "scene/material.hpp"

class FragmentShader {
   public:
    std::vector<Light> *lights = nullptr;
    vec3 eye_pos;

    FragmentShader(const Camera &camera);
    FragmentShader(const Camera &camera, std::vector<Light> &lights);
    vec3 shade(const vec3 &pos, const vec3 &normal, const vec2 &uv,
               const vec2 &duv, Material *material);
    vec3 blinn_phong(const vec3 &pos, const vec3 &normal, const vec2 &uv,
                     const vec2 &duv, Material *material);
    vec3 cel(const vec3 &pos, const vec3 &normal, const vec2 &uv,
             const vec2 &duv, Material *material);
    vec3 pbr(const vec3 &pos, const vec3 &normal, const vec2 &uv,
             const vec2 &duv, Material *material);
};

#endif