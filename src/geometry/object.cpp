#include "geometry/object.hpp"

#include <cmath>
#include <iostream>

#include "geometry/shape.hpp"
#include "geometry/triangle.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"
#include "scene/material.hpp"
#include "utils/functions.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#undef TINYOBJLOADER_IMPLEMENTATION

Object::Object(const vec3& pos, const vec3& rotation, const vec3& scale) {
    // Model transform

    model_transform.scale(scale);
    model_transform.rotation(rotation);
    model_transform.translation(pos);

    // Normal transform

    normal_transform.scale(scale);
    normal_transform.rotation(rotation);
}

void Object::do_model_transform() {
    // vertex
    for (auto& vertex : vertices) {
        vec4 pos = model_transform.transform(
            vec4(vertex->pos.x(), vertex->pos.y(), vertex->pos.z(), 1));
        vertex->pos = vec3(pos.x(), pos.y(), pos.z()) / pos.w();
        vertex->normal =
            normal_transform.transform(vertex->normal).normalized();
    }

    // normal
    for (auto& normal : normals) {
        *normal = normal_transform.transform(*normal).normalized();
    }
}

bool Object::load_model(const std::string& filename,
                        const std::string& basepath) {
    std::cout << "Load model: " << filename << std::endl;

    tinyobj::attrib_t t_attrib;
    std::vector<tinyobj::shape_t> t_shapes;
    std::vector<tinyobj::material_t> t_materials;

    std::string err;
    bool ret = tinyobj::LoadObj(
        &t_attrib, &t_shapes, &t_materials, &err, filename.c_str(),
        basepath.empty() ? nullptr : basepath.c_str(), true);

    if (!err.empty()) std::cerr << "ERR: " << err << std::endl;

    if (!ret) {
        printf("Failed to load/parse .obj.\n");
        return false;
    }

    std::cout << "Vertices count: " << t_attrib.vertices.size() / 3
              << std::endl;
    std::cout << "Shapes count: " << t_shapes.size() << std::endl;
    std::cout << "Materials cout: " << t_materials.size() << std::endl;

    vertices.reserve(t_attrib.vertices.size() / 3);
    for (size_t i = 0; i < t_attrib.vertices.size(); i += 3) {
        vertices.emplace_back(std::make_shared<Vertex>(
            vec3(t_attrib.vertices[i], t_attrib.vertices[i + 1],
                 t_attrib.vertices[i + 2])));
    }

    normals.reserve(t_attrib.normals.size() / 3);
    for (size_t i = 0; i < t_attrib.normals.size(); i += 3) {
        normals.emplace_back(std::make_shared<vec3>(t_attrib.normals[i],
                                                    t_attrib.normals[i + 1],
                                                    t_attrib.normals[i + 2]));
    }

    texcoords.reserve(t_attrib.texcoords.size() / 2);
    for (size_t i = 0; i < t_attrib.texcoords.size(); i += 2) {
        texcoords.emplace_back(std::make_shared<vec2>(
            t_attrib.texcoords[i], t_attrib.texcoords[i + 1]));
    }

    for (auto& t_material : t_materials) {
        auto material = std::make_shared<Material>();

        material->name = t_material.name;

        material->ambient =
            gamma_correction(vec3(t_material.ambient[0], t_material.ambient[1],
                                  t_material.ambient[2]),
                             2.2f);
        material->diffuse =
            gamma_correction(vec3(t_material.diffuse[0], t_material.diffuse[1],
                                  t_material.diffuse[2]),
                             2.2f);
        material->specular = gamma_correction(
            vec3(t_material.specular[0], t_material.specular[1],
                 t_material.specular[2]),
            2.2f);
        // material->transmittance =
        //     vec3(t_material.transmittance[0], t_material.transmittance[1],
        //          t_material.transmittance[2]);
        material->emission = gamma_correction(
            vec3(t_material.emission[0], t_material.emission[1],
                 t_material.emission[2]),
            2.2f);

        material->shininess = t_material.shininess;
        material->ior = t_material.ior;
        material->dissolve = t_material.dissolve;
        material->illum = t_material.illum;

        if (!t_material.ambient_texname.empty())
            material->ambient_texture =
                load_mipmap<vec3>(t_material.ambient_texname, basepath, false);

        if (!t_material.diffuse_texname.empty())
            material->diffuse_texture =
                load_mipmap<vec3>(t_material.diffuse_texname, basepath, false);

        if (!t_material.specular_texname.empty())
            material->specular_texture =
                load_mipmap<vec3>(t_material.specular_texname, basepath, false);

        if (!t_material.bump_texname.empty())
            material->bump_texture =
                load_mipmap<float>(t_material.bump_texname, basepath, true);

        if (!t_material.alpha_texname.empty()) {
            material->alpha_texture =
                load_texture_alpha(t_material.alpha_texname, basepath);
        }

        // material.specular_highlight_texname =
        //     t_material.specular_highlight_texname;
        // material.bump_texname = t_material.bump_texname;
        // material.bump_multiplier = t_material.bump_texopt.bump_multiplier;
        // material.alpha_texname = t_material.alpha_texname;
        // material.displacement_texname = t_material.displacement_texname;

        material->roughness = t_material.roughness;
        material->metallic = t_material.metallic;
        material->sheen = t_material.sheen;
        // material.clearcoat_thickness = t_material.clearcoat_thickness;
        // material.anisotropy = t_material.anisotropy;
        // material.anisotropy_rotation = t_material.anisotropy_rotation;

        // material.sheen_texname = t_material.sheen_texname;

        if (!t_material.emissive_texname.empty()) {
            material->emissive_texture =
                load_mipmap<vec3>(t_material.emissive_texname, basepath, false);
        }

        if (!t_material.roughness_texname.empty()) {
            material->roughness_texture = load_mipmap<float>(
                t_material.roughness_texname, basepath, true);
        }

        if (!t_material.metallic_texname.empty()) {
            material->metallic_texture =
                load_mipmap<float>(t_material.metallic_texname, basepath, true);
        }

        if (!t_material.normal_texname.empty()) {
            material->normal_texture =
                load_mipmap<vec3>(t_material.normal_texname, basepath, true);
        }

        materials.emplace_back(std::move(material));
    }

    size_t faces_count = 0;

    // For each shape
    for (auto& t_shape : t_shapes) {
        auto shape = Shape();

        size_t index_offset = 0;

        faces_count += t_shape.mesh.num_face_vertices.size();

        // For each face
        shape.triangles.reserve(t_shape.mesh.num_face_vertices.size());
        for (size_t f = 0; f < t_shape.mesh.num_face_vertices.size(); f++) {
            size_t fnum = t_shape.mesh.num_face_vertices[f];

            auto triangle = Triangle();

            triangle.vertices.reserve(3);
            // For each vertex in the face
            for (size_t v = 0; v < fnum; v++) {
                tinyobj::index_t idx = t_shape.mesh.indices[index_offset + v];
                triangle.vertices.emplace_back(vertices[idx.vertex_index]);
                if (idx.normal_index != -1) {
                    triangle.normals.emplace_back(normals[idx.normal_index]);
                    vertices[idx.vertex_index]->normal +=
                        *normals[idx.normal_index];
                }
                if (idx.texcoord_index != -1)
                    triangle.texcoords.emplace_back(
                        texcoords[idx.texcoord_index]);
            }

            if (t_shape.mesh.material_ids[f] != -1)
                triangle.material = materials[t_shape.mesh.material_ids[f]];

            // printf("  face[%ld].smoothing_group_id = %d\n",
            //        static_cast<long>(f),
            //        t_shape.mesh.smoothing_group_ids[f]);

            shape.triangles.emplace_back(std::move(triangle));

            index_offset += fnum;
        }

        shapes.emplace_back(std::move(shape));
    }

    for (auto& vertex : vertices) {
        vertex->normal = vertex->normal.normalized();
    }

    std::cout << "Faces count: " << faces_count << std::endl;

    return true;
}

std::shared_ptr<Texture<float>> Object::load_texture_alpha(
    const std::string& texname, const std::filesystem::path& base_path) {
    std::shared_ptr<Texture<float>> tex_ptr = nullptr;

    if (texture_alpha_map.find(texname) !=
        texture_alpha_map.end()) {  // loaded texture found
        tex_ptr = texture_alpha_map.at(texname);
    } else {  // not found
        std::cout << "Load alpha: " << texname << std::endl;
        tex_ptr = std::make_shared<Texture<float>>();
        texture_alpha_map.emplace(texname, tex_ptr);
        tex_ptr->read_alpha(base_path / texname);
    }
    return tex_ptr;
}

std::shared_ptr<Mipmap<float>> Object::load_mipmap_alpha(
    const std::string& texname, const std::filesystem::path& base_path) {
    std::shared_ptr<Mipmap<float>> mipmap_ptr = nullptr;

    if (mipmap_alpha_map.find(texname) !=
        mipmap_alpha_map.end()) {  // loaded mipmap found
        mipmap_ptr = mipmap_alpha_map.at(texname);
    } else {  // not found
        std::shared_ptr<Texture<float>> texture =
            load_texture_alpha(texname, base_path);
        mipmap_ptr = std::make_shared<Mipmap<float>>(texture);
    }
    return mipmap_ptr;
}