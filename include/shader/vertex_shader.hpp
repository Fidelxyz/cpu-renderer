#ifndef VERTEX_SHADER_H
#define VERTEX_SHADER_H

#include "camera.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"

class VertexShader {
   public:
    mat4 mvp_transform;
    mat4 screen_transform;

    VertexShader(Camera *camera);
    void shade(Vertex *vertex);
};

#endif