#pragma once
#include "math/Vec3.h"

struct Material {
    Vec3  ambient     = {0.15f, 0.15f, 0.15f};
    Vec3  diffuse     = {0.8f,  0.8f,  0.8f};
    Vec3  specular    = {0.5f,  0.5f,  0.5f};
    float shininess   = 32.0f;
    float restitution = 0.5f;

    static Material metal() {
        Material m;
        m.ambient     = {0.2f, 0.2f, 0.2f};
        m.diffuse     = {0.7f, 0.7f, 0.7f};
        m.specular    = {1.0f, 1.0f, 1.0f};
        m.shininess   = 128.0f;
        m.restitution = 0.8f;
        return m;
    }
    static Material wood() {
        Material m;
        m.ambient     = {0.1f, 0.08f, 0.05f};
        m.diffuse     = {0.9f, 0.9f,  0.9f};
        m.specular    = {0.1f, 0.1f,  0.1f};
        m.shininess   = 8.0f;
        m.restitution = 0.3f;
        return m;
    }
    static Material rubber() {
        Material m;
        m.ambient     = {0.1f, 0.1f, 0.1f};
        m.diffuse     = {1.0f, 1.0f, 1.0f};
        m.specular    = {0.0f, 0.0f, 0.0f};
        m.shininess   = 1.0f;
        m.restitution = 0.9f;
        return m;
    }
    static Material concrete() {
        Material m;
        m.ambient     = {0.15f, 0.15f, 0.15f};
        m.diffuse     = {0.6f,  0.6f,  0.6f};
        m.specular    = {0.15f, 0.15f, 0.15f};
        m.shininess   = 16.0f;
        m.restitution = 0.2f;
        return m;
    }
    static Material plastic() {
        return Material{};
    }
};

inline std::string materialName(const Material& m) {
    if (m.shininess >= 100.0f) return "metal";
    if (m.shininess <= 2.0f)   return "rubber";
    if (m.shininess <= 10.0f)  return "wood";
    if (m.shininess <= 20.0f)  return "concrete";
    return "plastic";
}

inline Material materialFromName(const std::string& name) {
    if (name == "metal")    return Material::metal();
    if (name == "wood")     return Material::wood();
    if (name == "rubber")   return Material::rubber();
    if (name == "concrete") return Material::concrete();
    return Material::plastic();
}
