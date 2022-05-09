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
    mat4 model_transform;

    std::vector<Vertex> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texcoords;

    std::vector<Shape> shapes;
    std::vector<Material> materials;

    Object(const vec3 &pos, const vec3 &rotation, const vec3 &scale);
    bool load_model(const std::string &filename, const std::string &basepath);
};

#endif