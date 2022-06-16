#include "shader/fragment_shader.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "utils/functions.hpp"

FragmentShader::FragmentShader(const Camera &camera) { eye_pos = camera.pos; }

FragmentShader::FragmentShader(const Camera &camera, std::vector<Light> &lights)
    : FragmentShader(camera) {
    this->lights = &lights;
}

float ramp(const float x) {
    constexpr float STEP1 = 0.3;
    constexpr float STEP2 = 0.6;
    constexpr float SMOTHNESS1 = 0.3;
    constexpr float SMOTHNESS2 = 0.1;
    constexpr float SHADOW = 0.4;
    constexpr float GAMMA = 0.7;
    constexpr float HIGHLIGHT = 0.9;

    static_assert(0.f <= STEP1 && STEP1 <= STEP2 && STEP2 <= 1.f);
    static_assert(0.f <= SHADOW && SHADOW <= GAMMA && GAMMA <= HIGHLIGHT &&
                  HIGHLIGHT <= 1.f);

    float step1_val = smoothstep(x, STEP1 - SMOTHNESS1, STEP1 + SMOTHNESS1);
    float step2_val = smoothstep(x, STEP2 - SMOTHNESS2, STEP2 + SMOTHNESS2);

    return SHADOW + step1_val * (GAMMA - SHADOW) +
           step2_val * (HIGHLIGHT - GAMMA);
}

float ramp_face(const float x) {
    constexpr float STEP = 0.5;
    constexpr float SMOTHNESS = 0.05;
    constexpr float SHADOW = 0.5;
    constexpr float HIGHLIGHT = 0.8;

    static_assert(0.f <= STEP && STEP <= 1.f);
    static_assert(0.f <= SHADOW && SHADOW <= HIGHLIGHT && HIGHLIGHT <= 1.f);

    float step_val = smoothstep(x, STEP - SMOTHNESS, STEP + SMOTHNESS);

    return SHADOW + step_val * (HIGHLIGHT - SHADOW);
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

    if (lights != nullptr) {
        for (auto &light : *lights) {
            vec3 light_dir = (light.pos - pos).normalized();
            float light_distance_squared = (light.pos - pos).squaredNorm();
            float reflected_intensity =
                light.intensity / light_distance_squared;

            if (material->name == "颜") {
                diffuse_shading +=
                    (light.color.cwiseProduct(diffuse)) *
                    ramp_face(reflected_intensity *
                              std::max(0.f, normal.dot(light_dir)));
            } else {
                diffuse_shading += (light.color.cwiseProduct(diffuse)) *
                                   ramp(reflected_intensity *
                                        std::max(0.f, normal.dot(light_dir)));
            }

            // vec3 h = (light_dir + eye_dir).normalized();
            // specular_shading +=
            //     reflection.cwiseProduct(specular) *
            //     std::pow(std::max(0.f, normal.dot(h)), material->shininess);
        }
    }

    return material->ambient + diffuse_shading + specular_shading;
}
