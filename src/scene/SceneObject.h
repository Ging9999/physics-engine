#pragma once
#include "physics/RigidBody.h"
#include "physics/Collider.h"
#include "math/Mat4.h"
#include "renderer/Mesh.h"
#include "renderer/Material.h"
#include <string>

class SceneObject {
public:
    RigidBody    rigidBody;
    Collider     collider;
    Mesh*        mesh       = nullptr;  // non-owning
    Vec3         color;
    Vec3         scale      = {1.0f, 1.0f, 1.0f};
    Material     material;
    unsigned int textureID  = 0;
    std::string  meshTag;               // "cube", "sphere", "obj", "plane"

    SceneObject(Mesh* mesh, Vec3 position, Vec3 color,
                Collider collider, float mass, bool isStatic,
                Material mat = Material::plastic(),
                unsigned int tex = 0,
                const std::string& tag = "");

    Mat4 getModelMatrix() const;
};
