#pragma once
#ifndef OBJECT_H
#define OBJECT_H

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "geometry/shape.hpp"
#include "global.hpp"
#include "scene/material.hpp"
#include "utils/transform.hpp"

class Object {
   public:
    Transform4 model_transform;
    Transform3 normal_transform;

    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<std::shared_ptr<vec3>> normals;
    std::vector<std::shared_ptr<vec2>> texcoords;

    std::unordered_map<std::string, std::shared_ptr<Texture<float>>>
        texture1_map;
    std::unordered_map<std::string, std::shared_ptr<Texture<vec3>>>
        texture3_map;
    std::unordered_map<std::string, std::shared_ptr<Mipmap<float>>> mipmap1_map;
    std::unordered_map<std::string, std::shared_ptr<Mipmap<vec3>>> mipmap3_map;

    std::vector<std::shared_ptr<Material>> materials;

    std::vector<Shape> shapes;

    Object(const vec3 &pos, const vec3 &rotation, const vec3 &scale);
    bool load_model(const std::string &filename, const std::string &basepath);
    void do_model_transform();

    template <typename T>
    std::shared_ptr<Texture<T>> load_texture(
        const std::string &texname, const std::filesystem::path &base_path);

    template <typename T>
    std::shared_ptr<Mipmap<T>> load_mipmap(
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

template <typename T>
std::shared_ptr<Mipmap<T>> Object::load_mipmap(
    const std::string &texname, const std::filesystem::path &base_path) {
    static_assert(std::is_same<T, float>::value ||
                  std::is_same<T, vec3>::value);

    std::unordered_map<std::string, std::shared_ptr<Mipmap<T>>> *mipmap_map;
    if constexpr (std::is_same<T, float>::value) {  // float
        mipmap_map = &mipmap1_map;
    } else {  // vec3
        mipmap_map = &mipmap3_map;
    }

    std::shared_ptr<Mipmap<T>> mipmap_ptr = nullptr;
    if (mipmap_map->find(texname) !=
        mipmap_map->end()) {  // loaded mipmap found
        mipmap_ptr = mipmap_map->at(texname);
    } else {  // not found
        std::shared_ptr<Texture<T>> texture =
            load_texture<T>(texname, base_path);
        mipmap_ptr = std::make_shared<Mipmap<T>>(texture);
    }
    return mipmap_ptr;
}

#endif