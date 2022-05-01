#ifndef FRAGMENT_SHADER_H
#define FRAGMENT_SHADER_H

#include <vector>

#include "global.hpp"
#include "light.hpp"

class FragmentShader {
   public:
    std::vector<Light *> lights;

    FragmentShader(std::vector<Light *> lights);
    vec3 shade(const vec3 &pos, const vec3 &normal);
};

#endif