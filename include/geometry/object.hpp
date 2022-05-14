#ifndef OBJECT_H
#define OBJECT_H

#include <Eigen/Core>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "geometry/shape.hpp"
#include "global.hpp"
#include "scene/material.hpp"

class Object {
   public:
    mat4 model_transform_matrix;
    mat3 normal_transform_matrix;

    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<std::shared_ptr<vec3>> normals;
    std::vector<std::shared_ptr<vec2>> texcoords;

    std::unordered_map<std::string, std::shared_ptr<Texture<float>>>
        texture1_map;
    std::unordered_map<std::string, std::shared_ptr<Texture<vec3>>>
        texture3_map;
    std::vector<std::shared_ptr<Material>> materials;

    std::vector<Shape> shapes;

    Object(const vec3 &pos, const vec3 &rotation, const vec3 &scale);
    bool load_model(const std::string &filename, const std::string &basepath);
    void model_transform();

    template <typename T>
    std::shared_ptr<Texture<T>> load_texture(
        const std::string &texname, const std::filesystem::path &base_path);
};

template <typename T>
std::shared_ptr<Texture<T>> Object::load_texture(
    const std::string &texname, const std::filesystem::path &base_path) {
    static_assert(std::is_same<T, float>::value ||
                  std::is_same<T, vec3>::value);

    std::unordered_map<std::string, std::shared_ptr<Texture<T>>> *texture_map;
    if constexpr (std::is_same<T, float>::value) {  // float
        texture_map = &texture1_map;
    } else {  // vec3
        texture_map = &texture3_map;
    }

    std::shared_ptr<Texture<T>> tex_ptr = nullptr;
    if (texture_map->find(texname) !=
        texture_map->end()) {  // loaded texture found
        tex_ptr = texture_map->at(texname);
    } else {  // not found
        std::cout << "Load texture: " << texname << std::endl;
        tex_ptr = std::make_shared<Texture<T>>();
        texture_map->emplace(texname, tex_ptr);
        tex_ptr->read_img(base_path / texname, false);
    }
    return tex_ptr;
}

#endif