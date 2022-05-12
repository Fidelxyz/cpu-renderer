#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include <opencv2/opencv.hpp>
#include <string>

#include "global.hpp"
#include "texture.hpp"

class Material {
   public:
    vec3 ambient = vec3(0, 0, 0);
    vec3 diffuse = vec3(0, 0, 0);
    vec3 specular = vec3(0, 0, 0);
    vec3 transmittance = vec3(0, 0, 0);
    vec3 emission = vec3(0, 0, 0);
    float shininess = 0;
    float ior = 0;
    float dissolve = 0;
    float illum = 0;

    // Texture<vec3> *ambient_texture = nullptr;
    std::shared_ptr<Texture<vec3>> diffuse_texture = nullptr;
    std::shared_ptr<Texture<vec3>> specular_texture = nullptr;

    // std::string ambient_texname;
    // std::string diffuse_texname;
    // std::string specular_texname;
    // std::string specular_highlight_texname;
    // std::string bump_texname;
    // float bump_multiplier = 0;
    // std::string alpha_texname;
    // std::string displacement_texname;

    // PBR

    // float roughness = 0;
    // float metallic = 0;
    // float sheen = 0;
    // float clearcoat_thickness = 0;
    // float anisotropy = 0;
    // float anisotropy_rotation = 0;

    // std::string emissive_texname;
    // std::string roughness_texname;
    // std::string metallic_texname;
    // std::string sheen_texname;
    // std::string normal_texname;

    Material();
};

#endif