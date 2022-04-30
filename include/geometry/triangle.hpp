#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <vector>

#include "geometry/material.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"

class Triangle {
   public:
    std::vector<Vertex *> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texcoords;
    Material *material = nullptr;

    Triangle();
};

#endif