#ifndef MESH_H
#define MESH_H

#include "global.hpp"

class Vertex {
   public:
    vec3 pos;
    vec3 view_pos;
    vec3 screen_pos;

    Vertex(const vec3 &pos);
};

#endif