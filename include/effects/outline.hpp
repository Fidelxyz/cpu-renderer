#pragma once
#ifndef OUTLINE_H
#define OUTLINE_H

#include "geometry/object.hpp"
#include "global.hpp"
#include "scene/camera.hpp"
#include "shader/fragment_shader.hpp"
#include "shader/vertex_shader.hpp"

namespace outline {
const float OUTLINE_WIDTH = 0.001;
const vec3 OUTLINE_COLOR = vec3(0, 0, 0);

// Object outline_geometry(const Object &object, const Camera &camera);

class OutlineVertexShader : public VertexShader {
   public:
    vec3 camera_pos;

    OutlineVertexShader(const Camera &camera);
    void shade(Vertex *vertex);
};

class OutlineFragmentShader : public FragmentShader {
   public:
    using FragmentShader::FragmentShader;
    vec3 shade(const vec3 &pos, const vec3 &normal, const vec2 &uv,
               const vec2 &duv, Material *material);
};
}  // namespace outline

#endif