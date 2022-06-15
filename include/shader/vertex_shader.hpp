#pragma once
#ifndef VERTEX_SHADER_H
#define VERTEX_SHADER_H

#include "geometry/vertex.hpp"
#include "global.hpp"
#include "scene/camera.hpp"
#include "utils/transform.hpp"

class VertexShader {
   public:
    PositionTransform position_transform;

    VertexShader();
    VertexShader(const Camera &camera);
    void shade(Vertex *vertex);
};

#endif