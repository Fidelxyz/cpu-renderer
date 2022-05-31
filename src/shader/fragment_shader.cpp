#include "shader/fragment_shader.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

FragmentShader::FragmentShader(const Camera &camera,
                               std::vector<Light> &lights) {
    this->lights = &lights;
    this->eye_pos = camera.pos;
}

vec3 FragmentShader::shade(const vec3 &pos, const vec3 &normal, const vec2 &uv,
                           const vec2 &duv, Material *material) {
    vec3 diffuse_shading = vec3(0, 0, 0);
    vec3 specular_shading = vec3(0, 0, 0);

    vec3 diffuse = material->diffuse_texture
                       ? material->diffuse_texture->sample(uv, duv)
                       : material->diffuse;

    vec3 specular = material->specular_texture
                        ? material->specular_texture->sample(uv, duv)
                        : material->specular;

    vec3 eye_dir = (eye_pos - pos).normalized();

    for (auto &light : *lights) {
        vec3 light_dir = (light.pos - pos).normalized();
        float light_distance_squared = (light.pos - pos).squaredNorm();
        float reflected_intensity = light.intensity / light_distance_squared;
        vec3 reflection = light.color * reflected_intensity;

        diffuse_shading += reflection.cwiseProduct(diffuse) *
                           std::max(0.f, normal.dot(light_dir));

        vec3 h = (light_dir + eye_dir).normalized();
        specular_shading +=
            reflection.cwiseProduct(specular) *
            std::pow(std::max(0.f, normal.dot(h)), material->shininess);
    }

    return material->ambient + diffuse_shading + specular_shading;
}