#ifndef OBJECT_H
#define OBJECT_H

#include <Eigen/Core>
#include <string>
#include <vector>

#include "geometry/material.hpp"
#include "geometry/shape.hpp"
#include "global.hpp"

class Object {
   public:
    std::vector<Vertex> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texcoords;

    std::vector<Shape> shapes;
    std::vector<Material> materials;

    Object();
    bool load_model(const char* filename, const char* basepath = NULL);
};

#endif