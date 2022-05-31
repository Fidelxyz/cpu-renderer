#pragma once
#ifndef MESH_H
#define MESH_H

#include "global.hpp"

class Vertex {
   public:
    vec3 pos;
    vec3 screen_pos;
    float w;

    Vertex(const vec3 &pos);
};

#endif