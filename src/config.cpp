#include "config.hpp"

#include <memory>
#include <unordered_map>

#include "camera.hpp"
#include "geometry/material.hpp"

Config::Config(const std::string &filename) : filename(filename) {}

Eigen::VectorXf Config::to_vector(const YAML::Node &yaml_array) {
    Eigen::VectorXf vector(yaml_array.size());
    for (size_t i = 0; i < yaml_array.size(); i++) {
        vector[i] = yaml_array[i].as<float>();
    }
    return vector;
}

bool Config::load_scene(Scene *scene) const {
    auto yaml_config = YAML::LoadFile(filename);

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

            if (yaml_material["diffuse-texname"])
                material->diffuse_texture = object.load_texture<vec3>(
                    yaml_material["diffuse-texname"].as<std::string>(),
                    base_path);

            if (yaml_material["specular-texname"])
                material->specular_texture = object.load_texture<vec3>(
                    yaml_material["specular-texname"].as<std::string>(),
                    base_path);

            object.materials.emplace_back(std::move(material));

            for (auto &shape : object.shapes) {
                for (auto &triangle : shape.triangles) {
                    triangle.material = object.materials.back();
                }
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
               to_vector(yaml_camera["look_dir"]).normalized(),
               to_vector(yaml_camera["up_dir"]).normalized(),
               yaml_camera["fov"].as<float>() * M_PI / 180.f,
               yaml_camera["near_plane"].as<float>(),
               yaml_camera["far_plane"].as<float>(),
               yaml_camera["width"].as<int>(), yaml_camera["height"].as<int>());

    return true;
}