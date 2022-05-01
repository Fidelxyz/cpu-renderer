#include "geometry/object.hpp"

#include <cassert>
#include <iostream>
#include <vector>

#include "geometry/material.hpp"
#include "geometry/shape.hpp"
#include "geometry/triangle.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#undef TINYOBJLOADER_IMPLEMENTATION

Object::Object() {}

bool Object::load_model(const char* filename, const char* basepath) {
    tinyobj::attrib_t t_attrib;
    std::vector<tinyobj::shape_t> t_shapes;
    std::vector<tinyobj::material_t> t_materials;

    std::string err;
    bool ret = tinyobj::LoadObj(&t_attrib, &t_shapes, &t_materials, &err,
                                filename, basepath, true);

    if (!err.empty()) std::cerr << "ERR: " << err << std::endl;

    if (!ret) {
        printf("Failed to load/parse .obj.\n");
        return false;
    }

    for (size_t i = 0; i < t_attrib.vertices.size(); i += 3) {
        vertices.push_back(
            Vertex(vec3(t_attrib.vertices[i], t_attrib.vertices[i + 1],
                        t_attrib.vertices[i + 2])));
    }

    for (size_t i = 0; i < t_attrib.normals.size(); i += 3) {
        normals.push_back(vec3(t_attrib.normals[i], t_attrib.normals[i + 1],
                               t_attrib.normals[i + 2]));
    }

    for (size_t i = 0; i < t_attrib.texcoords.size(); i += 2) {
        texcoords.push_back(
            vec2(t_attrib.texcoords[i], t_attrib.texcoords[i + 1]));
    }

    for (auto& t_material : t_materials) {
        auto material = Material(t_material.name);

        material.ambient = vec3(t_material.ambient[0], t_material.ambient[1],
                                t_material.ambient[2]);
        material.diffuse = vec3(t_material.diffuse[0], t_material.diffuse[1],
                                t_material.diffuse[2]);
        material.specular = vec3(t_material.specular[0], t_material.specular[1],
                                 t_material.specular[2]);
        material.transmittance =
            vec3(t_material.transmittance[0], t_material.transmittance[1],
                 t_material.transmittance[2]);
        material.emission = vec3(t_material.emission[0], t_material.emission[1],
                                 t_material.emission[2]);

        material.shininess = t_material.shininess;
        material.ior = t_material.ior;
        material.dissolve = t_material.dissolve;
        material.illum = t_material.illum;

        material.ambient_texname = t_material.ambient_texname;
        material.diffuse_texname = t_material.diffuse_texname;
        material.specular_texname = t_material.specular_texname;
        material.specular_highlight_texname =
            t_material.specular_highlight_texname;
        material.bump_texname = t_material.bump_texname;
        material.bump_multiplier = t_material.bump_texopt.bump_multiplier;
        material.alpha_texname = t_material.alpha_texname;
        material.displacement_texname = t_material.displacement_texname;

        material.roughness = t_material.roughness;
        material.metallic = t_material.metallic;
        material.sheen = t_material.sheen;
        material.clearcoat_thickness = t_material.clearcoat_thickness;
        material.clearcoat_thickness = t_material.clearcoat_thickness;
        material.anisotropy = t_material.anisotropy;
        material.anisotropy_rotation = t_material.anisotropy_rotation;

        material.emissive_texname = t_material.emissive_texname;
        material.roughness_texname = t_material.roughness_texname;
        material.metallic_texname = t_material.metallic_texname;
        material.sheen_texname = t_material.sheen_texname;
        material.normal_texname = t_material.normal_texname;

        // std::map<std::string, std::string>::const_iterator it(
        //     t_material.unknown_parameter.begin());
        // std::map<std::string, std::string>::const_iterator itEnd(
        //     t_material.unknown_parameter.end());

        // for (; it != itEnd; it++) {
        //     printf("  material.%s = %s\n", it->first.c_str(),
        //            it->second.c_str());
        // }
        // printf("\n");

        materials.push_back(std::move(material));
    }

    // For each shape
    for (auto& t_shape : t_shapes) {
        auto shape = Shape(t_shape.name);

        size_t index_offset = 0;

        // assert(t_shape.mesh.num_face_vertices.size() ==
        //        t_shape.mesh.material_ids.size());

        // assert(t_shape.mesh.num_face_vertices.size() ==
        //        t_shape.mesh.smoothing_group_ids.size());

        // printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
        //        static_cast<unsigned long>(
        //            t_shape.mesh.num_face_vertices.size()));

        // For each face
        for (size_t f = 0; f < t_shape.mesh.num_face_vertices.size(); f++) {
            size_t fnum = t_shape.mesh.num_face_vertices[f];
            assert(fnum == 3);

            auto triangle = Triangle();

            // For each vertex in the face
            for (size_t v = 0; v < fnum; v++) {
                tinyobj::index_t idx = t_shape.mesh.indices[index_offset + v];
                triangle.vertices.push_back(&vertices[idx.vertex_index]);
                if (idx.normal_index != -1)
                    triangle.normals.push_back(normals[idx.normal_index]);
                if (idx.texcoord_index != -1)
                    triangle.texcoords.push_back(texcoords[idx.texcoord_index]);
            }

            if (t_shape.mesh.material_ids[f] != -1)
                triangle.material = &materials[t_shape.mesh.material_ids[f]];

            // printf("  face[%ld].smoothing_group_id = %d\n",
            //        static_cast<long>(f),
            //        t_shape.mesh.smoothing_group_ids[f]);

            shape.triangles.push_back(std::move(triangle));

            index_offset += fnum;
        }

        // printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
        //        static_cast<unsigned long>(t_shape.mesh.tags.size()));
        // for (size_t t = 0; t < t_shape.mesh.tags.size(); t++) {
        //     printf("  tag[%ld] = %s ", static_cast<long>(t),
        //            t_shape.mesh.tags[t].name.c_str());
        //     printf(" ints: [");
        //     for (size_t j = 0; j < t_shape.mesh.tags[t].intValues.size();
        //          ++j) {
        //         printf("%ld", static_cast<long>(
        //                           t_shape.mesh.tags[t].intValues[j]));
        //         if (j < (t_shape.mesh.tags[t].intValues.size() - 1)) {
        //             printf(", ");
        //         }
        //     }
        //     printf("]");

        //     printf(" floats: [");
        //     for (size_t j = 0; j < t_shape.mesh.tags[t].floatValues.size();
        //          ++j) {
        //         printf("%f", static_cast<const double>(
        //                          t_shape.mesh.tags[t].floatValues[j]));
        //         if (j < (t_shape.mesh.tags[t].floatValues.size() - 1)) {
        //             printf(", ");
        //         }
        //     }
        //     printf("]");

        //     printf(" strings: [");
        //     for (size_t j = 0; j < t_shape.mesh.tags[t].stringValues.size();
        //          ++j) {
        //         printf("%s", t_shape.mesh.tags[t].stringValues[j].c_str());
        //         if (j < (t_shape.mesh.tags[t].stringValues.size() - 1)) {
        //             printf(", ");
        //         }
        //     }
        //     printf("]");
        //     printf("\n");
        // }

        shapes.push_back(std::move(shape));
    }

    return true;
}