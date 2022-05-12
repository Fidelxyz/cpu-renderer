#ifndef SHAPE_H
#define SHAPE_H

#include <string>
#include <vector>

#include "geometry/triangle.hpp"
#include "geometry/material.hpp"

class Shape {
   public:
    std::vector<Triangle> triangles;

    Shape();
};

#endif