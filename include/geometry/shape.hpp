#pragma once
#ifndef SHAPE_H
#define SHAPE_H

#include <vector>

#include "geometry/triangle.hpp"

class Shape {
   public:
    std::vector<Triangle> triangles;
};

#endif