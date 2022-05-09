#ifndef CONFIG_H
#define CONFIG_H

#include <Eigen/Core>
#include <string>

#include "scene.hpp"
#include "yaml-cpp/yaml.h"

class Config {
   public:
    Config(const std::string &filename);
    bool load_scene(Scene *scene) const;
    static Eigen::VectorXf to_vector(const YAML::Node &yaml_array);

   private:
    const std::string filename;
};

#endif