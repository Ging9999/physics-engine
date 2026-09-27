#pragma once
#include <string>
#include "math/Vec3.h"
#include "renderer/Mesh.h"
#include "scene/Scene.h"

class SceneSaver {
public:
    static bool save(const std::string& filepath, const Scene& scene, const Vec3& lightPos);
    static bool load(const std::string& filepath, Scene& scene, Vec3& lightPos,
                     Mesh& cubeMesh, Mesh& sphereMesh, Mesh& objMesh);
};
