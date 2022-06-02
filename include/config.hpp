#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include <Eigen/Core>
#include <string>

#include "scene/scene.hpp"
#include "yaml-cpp/yaml.h"

class Config {
   private:
    YAML::Node yaml_config;

   public:
    Config(const std::string &filename);
    bool load_scene(Scene *scene) const;
    bool load_threads_num(int *threads_num) const;

   private:
    static Eigen::VectorXf to_vector(const YAML::Node &yaml_array);
};

#endif