#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include "global.hpp"

class Light {
   public:
    vec3 pos;
    vec3 color;
    float intensity;
    Light(const vec3 &pos, const vec3 &color, const float intensity);
};

#endif