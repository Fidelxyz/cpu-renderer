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

///////////////////////////
/// Blinn-Phong Shading ///
///////////////////////////

vec3 FragmentShader::blinn_phong(const vec3 &pos, const vec3 &normal,
                                 const vec2 &uv, const vec2 &duv,
                                 Material *material) {
    vec3 diffuse_shading = vec3(0, 0, 0);
    vec3 specular_shading = vec3(0, 0, 0);

    vec3 ambient = material->ambient_texture
                       ? material->ambient_texture->sample(uv, duv)
                       : material->ambient;

    vec3 diffuse = material->diffuse_texture
                       ? material->diffuse_texture->sample(uv, duv)
                       : material->diffuse;

    vec3 specular = material->specular_texture
                        ? material->specular_texture->sample(uv, duv)
                        : material->specular;

    vec3 view_dir = (eye_pos - pos).normalized();  // shading point -> eye
    for (auto &light : *lights) {
        vec3 light_dir =
            (light.pos - pos).normalized();  // shading point -> light
        float reflected_intensity =
            light.intensity / (light.pos - pos).squaredNorm();
        vec3 reflection = light.color * reflected_intensity;

        diffuse_shading += reflection * std::max(0.f, normal.dot(light_dir));

        vec3 h = (light_dir + view_dir).normalized();
        specular_shading += reflection * std::pow(std::max(0.f, normal.dot(h)),
                                                  material->shininess);
    }

    return ambient + diffuse_shading.cwiseProduct(diffuse) +
           specular_shading.cwiseProduct(specular);
}

///////////////////
/// Cel Shading ///
///////////////////

float ramp(const float x) {
    constexpr float STEP1 = 0.4;
    constexpr float STEP2 = 0.9;
    constexpr float SMOTHNESS1 = 0.05;
    constexpr float SMOTHNESS2 = 0.1;
    constexpr float SHADOW = 0.3;
    constexpr float GAMMA = 0.6;
    constexpr float HIGHLIGHT = 0.8;

    static_assert(0.f <= STEP1 && STEP1 <= STEP2 && STEP2 <= 1.f);
    static_assert(0.f <= SHADOW && SHADOW <= GAMMA && GAMMA <= HIGHLIGHT &&
                  HIGHLIGHT <= 1.f);

    float step1_val = smoothstep(STEP1 - SMOTHNESS1, STEP1 + SMOTHNESS1, x);
    float step2_val = smoothstep(STEP2 - SMOTHNESS2, STEP2 + SMOTHNESS2, x);

    return SHADOW + step1_val * (GAMMA - SHADOW) +
           step2_val * (HIGHLIGHT - GAMMA);
}

float ramp_face(const float x) {
    constexpr float STEP = 0.4;
    constexpr float SMOTHNESS = 0.05;
    constexpr float SHADOW = 0.4;
    constexpr float HIGHLIGHT = 0.8;

    static_assert(0.f <= STEP && STEP <= 1.f);
    static_assert(0.f <= SHADOW && SHADOW <= HIGHLIGHT && HIGHLIGHT <= 1.f);

    float step_val = smoothstep(STEP - SMOTHNESS, STEP + SMOTHNESS, x);

    return SHADOW + step_val * (HIGHLIGHT - SHADOW);
}

vec3 FragmentShader::cel(const vec3 &pos, const vec3 &normal, const vec2 &uv,
                         const vec2 &duv, Material *material) {
    vec3 diffuse_shading = vec3(0, 0, 0);

    vec3 ambient = material->ambient_texture
                       ? material->ambient_texture->sample(uv, duv)
                       : material->ambient;

    vec3 diffuse = material->diffuse_texture
                       ? material->diffuse_texture->sample(uv, duv)
                       : material->diffuse;

    for (auto &light : *lights) {
        vec3 light_dir = (light.pos - pos).normalized();
        float intensity = light.intensity / (light.pos - pos).squaredNorm();

        float cos_l = normal.dot(light_dir);
        if (cos_l < 0.f) continue;

        diffuse_shading += light.color * intensity * cos_l / M_PI;
    }

    float lighting_luminance = std::min(luminance(diffuse_shading), 1.f);
    float ramped_luminance = material->name == "颜" || material->name == "面1"
                                 ? ramp_face(lighting_luminance)
                                 : ramp(lighting_luminance);
    float factor = lighting_luminance < EPS
                       ? ramped_luminance
                       : ramped_luminance / lighting_luminance;
    diffuse_shading *= factor;

    return ambient + diffuse_shading.cwiseProduct(diffuse);
}

///////////
/// PBR ///
///////////

float square(const float x) { return x * x; }

float pow5(const float x) { return x * x * x * x * x; }

vec3 FragmentShader::pbr(const vec3 &pos, const vec3 &normal, const vec2 &uv,
                         const vec2 &duv, Material *material) {
    vec3 shading = vec3(0, 0, 0);

    vec3 base_color = material->diffuse_texture
                          ? material->diffuse_texture->sample(uv, duv)
                          : material->diffuse;
    float roughness = material->roughness_texture
                          ? material->roughness_texture->sample(uv, duv)
                          : material->roughness;
    float metallic = material->metallic_texture
                         ? material->metallic_texture->sample(uv, duv)
                         : material->metallic;

    float occlusion =
        material->bump_texture ? material->bump_texture->sample(uv, duv) : 1.f;

    if (material->emissive_texture) {
        shading += material->emissive_texture->sample(uv, duv) * material->ior;
    }

    vec3 view_dir = (eye_pos - pos).normalized();  // shading point -> eye

    for (auto &light : *lights) {
        float intensity = light.intensity / (light.pos - pos).squaredNorm();

        vec3 light_dir =
            (light.pos - pos).normalized();  // shading point -> light

        float cos_l = std::max(normal.dot(light_dir), 0.f);
        float cos_v = std::max(normal.dot(view_dir), 0.f);

        vec3 h = (light_dir + view_dir).normalized();  // half vector
        float cos_d = std::max(light_dir.dot(h), 0.f);

        // Diffuse

        float fd90 = 0.5f + 2.f * roughness * square(cos_d);
        vec3 diffuse = base_color / M_PI *
                       (1.f + (fd90 - 1.f) * std::pow(1.f - cos_l, 5)) *
                       (1.f + (fd90 - 1.f) * std::pow(1.f - cos_v, 5));

        // Specular: Cook-Torrance

        // D: GGX/Trowbridge-Reitz
        float alpha = square(roughness);
        float alpha_squared = square(alpha);
        float d =
            alpha_squared /
            (M_PI * square(square(normal.dot(h)) * (alpha_squared - 1) + 1));

        // G: Smith
        float k = square((roughness + 1.0) / 2.0) / 2.0;
        float g1 = cos_v / (cos_v * (1 - k) + k);
        float g2 = cos_l / (cos_l * (1 - k) + k);
        float g = g1 * g2;

        // F: Fresnel
        vec3 f0 = vec3(0.04, 0.04, 0.04);
        f0 = (1 - metallic) * f0 + metallic * base_color;
        vec3 f = f0 + (vec3(1, 1, 1) - f0) * pow5(1 - cos_v);

        vec3 specular = (d * g * f) / (4 * cos_v * cos_l + EPS);
        // add EPS to avoid division by 0

        shading +=
            (intensity * light.color).cwiseProduct(diffuse + specular) * cos_l;
    }

    shading = shading * (0.5f + 0.5f * occlusion);

    return shading;
}

vec3 FragmentShader::shade(const vec3 &pos, const vec3 &normal, const vec2 &uv,
                           const vec2 &duv, Material *material) {
    if (material->shading_type == "pbr") {
        return pbr(pos, normal, uv, duv, material);
    } else if (material->shading_type == "cel") {
        return cel(pos, normal, uv, duv, material);
    } else {  // material->shading_type == default
        return blinn_phong(pos, normal, uv, duv, material);
    }
}
