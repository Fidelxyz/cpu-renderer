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
    constexpr float SHADOW = 0.6;
    constexpr float HIGHLIGHT = 0.9;

    static_assert(0.f <= STEP && STEP <= 1.f);
    static_assert(0.f <= SHADOW && SHADOW <= HIGHLIGHT && HIGHLIGHT <= 1.f);

    float step_val = smoothstep(x, STEP - SMOTHNESS, STEP + SMOTHNESS);

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
        float reflected_intensity =
            light.intensity / (light.pos - pos).squaredNorm();

        if (material->name == "颜" || material->name == "面1") {
            diffuse_shading +=
                light.color * ramp_face(reflected_intensity *
                                        std::max(0.f, normal.dot(light_dir)));
        } else {
            diffuse_shading +=
                light.color * ramp(reflected_intensity *
                                   std::max(0.f, normal.dot(light_dir)));
        }
    }

    return ambient + diffuse_shading.cwiseProduct(diffuse);
}

///////////
/// PBR ///
///////////

float square(const float x) { return x * x; }

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

    vec3 view_dir = (eye_pos - pos).normalized();  // shading point -> eye

    for (auto &light : *lights) {
        float intensity = light.intensity / (light.pos - pos).squaredNorm();

        vec3 light_dir =
            (light.pos - pos).normalized();            // shading point -> light
        vec3 h = (light_dir + view_dir).normalized();  // half vector

        float cos_l = normal.dot(light_dir);
        if (cos_l <= 0.f) continue;
        float cos_v = normal.dot(view_dir);
        if (cos_v <= 0.f) continue;

        // float r0 = square((1 - material->ior) / (1 + material->ior));
        float r0 = square((1 - material->ior) / (1 + material->ior));
        float fresnel = r0 + (1 - r0) * (1 - cos_v);

        // Diffuse

        vec3 diffuse = (1.f - fresnel) * base_color / M_PI;

        // Specular: Cook-Torrance

        // D: GGX/Trowbridge-Reitz
        float alpha = square(roughness);
        float alpha_squared = square(alpha);
        float d =
            alpha_squared /
            (M_PI * square(square(normal.dot(h)) * (alpha_squared - 1) + 1));

        // G: Smith
        float k = alpha_squared / 2;
        float g1 = cos_v / (cos_v * (1 - k) + k);
        float g2 = cos_l / (cos_l * (1 - k) + k);
        float g = g1 * g2;

        // F: Fresnel
        vec3 f0 = vec3(r0, r0, r0);
        f0 = (1 - metallic) * f0 + metallic * base_color;
        vec3 f = f0 + (vec3(1, 1, 1) - f0) * (1 - cos_v);

        vec3 specular = (d * g * f) / (4 * cos_v * cos_l);

        shading +=
            intensity * light.color.cwiseProduct(diffuse + specular) * cos_l;
    }

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
