#include "config.hpp"

#include <iostream>
#include <memory>
#include <unordered_map>

#include "global.hpp"
#include "scene/camera.hpp"
#include "scene/material.hpp"

Config::Config(const std::string &filename) {
    std::cout << "Load scene from config: " << filename << std::endl;
    yaml_config = YAML::LoadFile(filename);
}

Eigen::VectorXf Config::to_vector(const YAML::Node &yaml_array) {
    Eigen::VectorXf vector(yaml_array.size());
    for (size_t i = 0; i < yaml_array.size(); i++) {
        vector[i] = yaml_array[i].as<float>();
    }
    return vector;
}

bool Config::load_scene(Scene *scene) const {
    // objects
    for (auto yaml_object : yaml_config["objects"]) {
        auto base_path =
            std::filesystem::path(yaml_object["basepath"].as<std::string>());

        auto object = Object(to_vector(yaml_object["pos"]),
                             to_vector(yaml_object["rotation"]),
                             to_vector(yaml_object["scale"]));

        if (!object.load_model(yaml_object["path"].as<std::string>(),
                               yaml_object["basepath"].as<std::string>()))
            return false;

        // objects.material
        auto yaml_material = yaml_object["material"];
        if (yaml_material) {
            std::shared_ptr<Material> material = std::make_shared<Material>();
            if (yaml_material["ambient"])
                material->ambient = to_vector(yaml_material["ambient"]);
            if (yaml_material["specular"])
                material->specular = to_vector(yaml_material["specular"]);
            if (yaml_material["diffuse"])
                material->diffuse = to_vector(yaml_material["diffuse"]);
            if (yaml_material["shininess"])
                material->shininess = yaml_material["shininess"].as<float>();
            if (yaml_material["ior"])
                material->ior = yaml_material["ior"].as<float>();

            if (yaml_material["ambient-texname"])
                material->ambient_texture = object.load_mipmap<vec3>(
                    yaml_material["ambient-texname"].as<std::string>(),
                    base_path, false);

            if (yaml_material["diffuse-texname"])
                material->diffuse_texture = object.load_mipmap<vec3>(
                    yaml_material["diffuse-texname"].as<std::string>(),
                    base_path, false);

            if (yaml_material["specular-texname"])
                material->specular_texture = object.load_mipmap<vec3>(
                    yaml_material["specular-texname"].as<std::string>(),
                    base_path, false);

            if (yaml_material["alpha-texname"])
                material->alpha_texture = object.load_mipmap_alpha(
                    yaml_material["alpha-texname"].as<std::string>(),
                    base_path);

            if (yaml_material["roughness"])
                material->roughness = yaml_material["roughness"].as<float>();
            if (yaml_material["metallic"])
                material->metallic = yaml_material["metallic"].as<float>();
            if (yaml_material["sheen"])
                material->sheen = yaml_material["sheen"].as<float>();

            if (yaml_material["normal-texname"])
                material->normal_texture = object.load_mipmap<vec3>(
                    yaml_material["normal-texname"].as<std::string>(),
                    base_path, true);

            object.materials.emplace_back(std::move(material));

            for (auto &shape : object.shapes) {
                for (auto &triangle : shape.triangles) {
                    triangle.material = object.materials.back();
                }
            }
        }

        if (yaml_object["shading-type"]) {
            for (auto &material : object.materials) {
                material->shading_type =
                    yaml_object["shading-type"].as<std::string>();
            }
        }

        scene->objects.emplace_back(std::move(object));
    }

    // lights
    for (auto yaml_light : yaml_config["lights"]) {
        scene->lights.emplace_back(to_vector(yaml_light["pos"]),
                                   to_vector(yaml_light["color"]),
                                   yaml_light["intensity"].as<float>());
    }

    // camera
    auto yaml_camera = yaml_config["camera"];
    scene->camera =
        Camera(to_vector(yaml_camera["pos"]),
               to_vector(yaml_camera["look-dir"]).normalized(),
               to_vector(yaml_camera["up-dir"]).normalized(),
               yaml_camera["fov"].as<float>() * M_PI / 180.f,
               yaml_camera["near-plane"].as<float>(),
               yaml_camera["far-plane"].as<float>(),
               yaml_camera["width"].as<int>(), yaml_camera["height"].as<int>(),
               yaml_camera["relax-view-culling-factor"].as<float>());

    // other
    if (yaml_config["background-color"])
        scene->background_color = to_vector(yaml_config["background-color"]);
    if (yaml_config["enable-rimlight"])
        scene->enable_rimlight = yaml_config["enable-rimlight"].as<bool>();

    return true;
}

bool Config::load_threads_num(int *threads_num) const {
    if (!yaml_config["threads-num"]) {
        std::cout << "[Warning] threads-num is not found in config."
                  << std::endl;
        return false;
    }

    *threads_num = yaml_config["threads-num"].as<int>();

    if (*threads_num < 1) {
        std::cout << "[Warning] threads-num cannot be less than 1."
                  << std::endl;
        return false;
    }

    return true;
}