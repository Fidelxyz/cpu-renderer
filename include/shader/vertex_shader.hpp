#ifndef VERTEX_SHADER_H
#define VERTEX_SHADER_H

#include "scene/camera.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"

class VertexShader {
   public:
    mat4 transform;

    VertexShader(const Camera &camera);
    void shade(Vertex *vertex);
};

#endif