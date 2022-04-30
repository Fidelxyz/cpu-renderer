#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>

#include "global.hpp"

class Material {
   public:
    std::string name;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 transmittance;
    vec3 emission;
    float shininess;
    float ior;
    float dissolve;
    float illum;

    std::string ambient_texname;
    std::string diffuse_texname;
    std::string specular_texname;
    std::string specular_highlight_texname;
    std::string bump_texname;
    float bump_multiplier;
    std::string alpha_texname;
    std::string displacement_texname;

    // PBR
    
    float roughness;
    float metallic;
    float sheen;
    float clearcoat_thickness;
    float anisotropy;
    float anisotropy_rotation;

    std::string emissive_texname;
    std::string roughness_texname;
    std::string metallic_texname;
    std::string sheen_texname;
    std::string normal_texname;

    Material(const std::string &name);
};

#endif