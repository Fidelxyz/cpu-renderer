#include "geometry/object.hpp"

#include <cmath>
#include <iostream>

#include "geometry/shape.hpp"
#include "geometry/triangle.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"
#include "scene/material.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#undef TINYOBJLOADER_IMPLEMENTATION

Object::Object(const vec3& pos, const vec3& rotation, const vec3& scale) {
    vec3 rotation_arc = rotation * M_PI / 180.f;
    float sin_x = std::sin(rotation_arc.x());
    float sin_y = std::sin(rotation_arc.y());
    float sin_z = std::sin(rotation_arc.z());
    float cos_x = std::cos(rotation_arc.x());
    float cos_y = std::cos(rotation_arc.y());
    float cos_z = std::cos(rotation_arc.z());

    // Model transform

    mat4 model_translation;
    // clang-format off
    model_translation << 1, 0, 0, pos.x(),
                         0, 1, 0, pos.y(),
                         0, 0, 1, pos.z(),
                         0, 0, 0, 1;
    // clang-format on

    mat4 model_rotation_x, model_rotation_y, model_rotation_z;
    // clang-format off
    model_rotation_x << 1, 0,     0,      0,
                        0, cos_x, -sin_x, 0,
                        0, sin_x, cos_x,  0,
                        0, 0,     0,      1;

    model_rotation_y << cos_y,  0, sin_y, 0,
                        0,      1, 0,     0,
                        -sin_y, 0, cos_y, 0,
                        0,      0, 0,     1;

    model_rotation_z << cos_z, -sin_z, 0, 0,
                        sin_z, cos_z,  0, 0,
                        0,     0,      1, 0,
                        0,     0,      0, 1;
    // clang-format on
    mat4 model_rotation =
        model_rotation_z * model_rotation_y * model_rotation_x;

    mat4 model_scale;
    // clang-format off
    model_scale << scale.x(), 0, 0, 0,
                   0, scale.y(), 0, 0,
                   0, 0, scale.z(), 0,
                   0, 0, 0, 1;
    // clang-format on

    model_transform_matrix = model_translation * model_rotation * model_scale;

    // Normal transform

    mat3 normal_rotation_x, normal_rotation_y, normal_rotation_z;
    // clang-format off
    normal_rotation_x << 1, 0,     0,      
                         0, cos_x, -sin_x,
                         0, sin_x, cos_x;

    normal_rotation_y << cos_y,  0, sin_y,
                         0,      1, 0,    
                         -sin_y, 0, cos_y;

    normal_rotation_z << cos_z, -sin_z, 0,
                         sin_z, cos_z,  0,
                         0,     0,      1;
    // clang-format on
    mat3 normal_rotation =
        normal_rotation_z * normal_rotation_y * normal_rotation_x;

    mat3 normal_scale;
    // clang-format off
    normal_scale << scale.x(), 0, 0,
                    0, scale.y(), 0,
                    0, 0, scale.z();
    // clang-format on

    normal_transform_matrix = normal_rotation * normal_scale;
}

void Object::model_transform() {
    // pos
    for (auto& vertex : vertices) {
        vec4 pos = model_transform_matrix *
                   vec4(vertex->pos.x(), vertex->pos.y(), vertex->pos.z(), 1);
        vertex->pos = vec3(pos.x(), pos.y(), pos.z());
    }

    // normal
    for (auto& normal : normals) {
        *normal = (normal_transform_matrix * *normal).normalized();
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
    std::cout << "Shapes count: " << shapes.size() << std::endl;
    std::cout << "Materials cout: " << materials.size() << std::endl;

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

        material->ambient = vec3(t_material.ambient[0], t_material.ambient[1],
                                 t_material.ambient[2]);
        material->diffuse = vec3(t_material.diffuse[0], t_material.diffuse[1],
                                 t_material.diffuse[2]);
        material->specular =
            vec3(t_material.specular[0], t_material.specular[1],
                 t_material.specular[2]);
        material->transmittance =
            vec3(t_material.transmittance[0], t_material.transmittance[1],
                 t_material.transmittance[2]);
        material->emission =
            vec3(t_material.emission[0], t_material.emission[1],
                 t_material.emission[2]);

        material->shininess = t_material.shininess;
        material->ior = t_material.ior;
        material->dissolve = t_material.dissolve;
        material->illum = t_material.illum;

        if (!t_material.ambient_texname.empty())
            material->ambient_texture =
                load_texture<vec3>(t_material.ambient_texname, basepath);

        if (!t_material.diffuse_texname.empty())
            material->diffuse_texture =
                load_texture<vec3>(t_material.diffuse_texname, basepath);

        if (!t_material.specular_texname.empty())
            material->specular_texture =
                load_texture<vec3>(t_material.specular_texname, basepath);

        // material.specular_highlight_texname =
        //     t_material.specular_highlight_texname;
        // material.bump_texname = t_material.bump_texname;
        // material.bump_multiplier = t_material.bump_texopt.bump_multiplier;
        // material.alpha_texname = t_material.alpha_texname;
        // material.displacement_texname = t_material.displacement_texname;

        // material.roughness = t_material.roughness;
        // material.metallic = t_material.metallic;
        // material.sheen = t_material.sheen;
        // material.clearcoat_thickness = t_material.clearcoat_thickness;
        // material.clearcoat_thickness = t_material.clearcoat_thickness;
        // material.anisotropy = t_material.anisotropy;
        // material.anisotropy_rotation = t_material.anisotropy_rotation;

        // material.emissive_texname = t_material.emissive_texname;
        // material.roughness_texname = t_material.roughness_texname;
        // material.metallic_texname = t_material.metallic_texname;
        // material.sheen_texname = t_material.sheen_texname;
        // material.normal_texname = t_material.normal_texname;

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
                if (idx.normal_index != -1)
                    triangle.normals.emplace_back(normals[idx.normal_index]);
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

    std::cout << "Faces count: " << faces_count << std::endl;

    return true;
}