#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include <string>

#include "global.hpp"
#include "texture/mipmap.hpp"
#include "texture/texture.hpp"

class Material {
   public:
    std::string name;
    std::string shading_type = "default";

    vec3 ambient = vec3(0, 0, 0);
    vec3 diffuse = vec3(0, 0, 0);
    vec3 specular = vec3(0, 0, 0);
    vec3 transmittance = vec3(0, 0, 0);
    vec3 emission = vec3(0, 0, 0);
    float shininess = 0;
    float ior = 0;
    float dissolve = 0;
    float illum = 0;

    std::shared_ptr<Mipmap<vec3>> ambient_texture;
    std::shared_ptr<Mipmap<vec3>> diffuse_texture;
    std::shared_ptr<Mipmap<vec3>> specular_texture;
    std::shared_ptr<Mipmap<float>> alpha_texture;

    // std::string ambient_texname;
    // std::string diffuse_texname;
    // std::string specular_texname;
    // std::string specular_highlight_texname;
    // std::string bump_texname;
    // float bump_multiplier = 0;
    // std::string alpha_texname;
    // std::string displacement_texname;

    // PBR

    float roughness = 0;
    float metallic = 0;
    float sheen = 0;
    // float clearcoat_thickness = 0;
    // float anisotropy = 0;
    // float anisotropy_rotation = 0;

    std::shared_ptr<Mipmap<float>> roughness_texture;
    std::shared_ptr<Mipmap<float>> metallic_texture;
    std::shared_ptr<Mipmap<vec3>> normal_texture;

    // std::string emissive_texname;
    // std::string roughness_texname;
    // std::string metallic_texname;
    // std::string sheen_texname;
    // std::string normal_texname;
};

#endif