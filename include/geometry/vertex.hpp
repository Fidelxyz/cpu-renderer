#pragma once
#ifndef MESH_H
#define MESH_H

#include "global.hpp"

class Vertex {
   public:
    vec3 pos;
    vec3 screen_pos;
    double w;
    vec3 normal = vec3(0, 0, 0);

    Vertex();
    Vertex(const vec3 &pos);
};

#endif