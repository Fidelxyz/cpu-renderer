#include "light/light.hpp"

Light::Light(const vec3 &pos, const vec3 &color, const float intensity) {
    this->pos = pos;
    this->color = color;
    this->intensity = intensity;
}