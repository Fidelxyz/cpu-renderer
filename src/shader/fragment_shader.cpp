#include "shader/fragment_shader.hpp"

FragmentShader::FragmentShader(std::vector<Light *> lights) {
    this->lights = lights;
}

vec3 shade(vec3 pos, vec3 normal) { return vec3(0, 0, 0); }