#ifndef LIGHT_H
#define LIGHT_H

#include "global.hpp"

class Light {
   public:
    vec3 pos;
    Light(const vec3 pos);
};

#endif