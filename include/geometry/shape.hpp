#ifndef SHAPE_H
#define SHAPE_H

#include <string>
#include <vector>

#include "geometry/triangle.hpp"
#include "geometry/material.hpp"

class Shape {
   public:
    std::string name;
    std::vector<Triangle> triangles;

    Shape(const std::string &name);
};

#endif