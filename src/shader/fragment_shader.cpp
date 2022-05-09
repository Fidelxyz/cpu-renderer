#include "shader/fragment_shader.hpp"

#include <algorithm>
#include <cmath>

FragmentShader::FragmentShader(const Camera &camera,
                               std::vector<Light> &lights) {
    this->lights = &lights;
    this->eye_pos = camera.pos;
}

vec3 FragmentShader::shade(const vec3 &pos, const vec3 &normal,
                           Material *material) {
    vec3 diffuse = vec3(0, 0, 0);
    vec3 specular = vec3(0, 0, 0);

    vec3 eye_dir = (eye_pos - pos).normalized();

    for (auto &light : *lights) {
        vec3 light_dir = (light.pos - pos).normalized();
        float light_distance_squared = (light.pos - pos).squaredNorm();
        float reflected_intensity = light.intensity / light_distance_squared;

        diffuse += light.color.cwiseProduct(material->diffuse) *
                   reflected_intensity * std::max(0.f, normal.dot(light_dir));
        // TODO Light color

        vec3 h = (light_dir + eye_dir).normalized();
        specular += light.color.cwiseProduct(material->specular) *
                    reflected_intensity *
                    std::pow(std::max(0.f, normal.dot(h)), material->shininess);
    }

    return material->ambient + diffuse + specular;
}