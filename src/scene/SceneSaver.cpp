#include "scene/SceneSaver.h"
#include "scene/SceneObject.h"
#include "physics/Collider.h"
#include "renderer/Material.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

bool SceneSaver::save(const std::string& filepath, const Scene& scene, const Vec3& lightPos) {
    // Ensure parent directory exists
    std::filesystem::path p(filepath);
    if (p.has_parent_path())
        std::filesystem::create_directories(p.parent_path());

    std::ofstream f(filepath);
    if (!f.is_open()) {
        std::cerr << "SceneSaver: cannot write " << filepath << "\n";
        return false;
    }

    f << "# PhysicsEngine Scene File\n";
    f << "# type posX posY posZ scaleX scaleY scaleZ colorR colorG colorB material isStatic\n";
    f << std::fixed;
    f.precision(4);

    f << "light " << lightPos.x << " " << lightPos.y << " " << lightPos.z << "\n";

    for (auto& obj : scene.objects) {
        std::string tag = obj.meshTag.empty() ? "cube" : obj.meshTag;
        auto& rb = obj.rigidBody;
        auto& s  = obj.scale;
        auto& c  = obj.color;
        std::string mat = materialName(obj.material);
        f << tag
          << " " << rb.position.x << " " << rb.position.y << " " << rb.position.z
          << " " << s.x << " " << s.y << " " << s.z
          << " " << c.x << " " << c.y << " " << c.z
          << " " << mat
          << " " << (rb.isStatic ? 1 : 0)
          << "\n";
    }

    std::cout << "Scene saved to " << filepath << " (" << scene.objects.size() << " objects)\n";
    return true;
}

bool SceneSaver::load(const std::string& filepath, Scene& scene, Vec3& lightPos,
                       Mesh& cubeMesh, Mesh& sphereMesh, Mesh& objMesh) {
    std::ifstream f(filepath);
    if (!f.is_open()) {
        std::cerr << "SceneSaver: cannot read " << filepath << "\n";
        return false;
    }

    scene.clear();
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "light") {
            iss >> lightPos.x >> lightPos.y >> lightPos.z;
            continue;
        }

        float px, py, pz, sx, sy, sz, cr, cg, cb;
        std::string matName;
        int isStaticInt;
        iss >> px >> py >> pz >> sx >> sy >> sz >> cr >> cg >> cb >> matName >> isStaticInt;

        Vec3 pos{px,py,pz}, scale{sx,sy,sz}, color{cr,cg,cb};
        Material mat  = materialFromName(matName);
        bool  isStat  = (isStaticInt != 0);

        Mesh*    mesh = &cubeMesh;
        Collider col  = Collider::createAABB(scale);
        if (type == "sphere")       { mesh = &sphereMesh; col = Collider::createSphere(scale.x); }
        else if (type == "obj")     { mesh = &objMesh;    col = Collider::createAABB(scale); }
        else if (type == "plane_floor" || type == "plane_wall_x" || type == "plane_wall_z") {
            mesh = &cubeMesh; col = Collider::createAABB(scale);
        }

        SceneObject obj(mesh, pos, color, col, mat.restitution > 0 ? 1.0f : 0.0f, isStat,
                        mat, 0, type);
        obj.scale = scale;
        scene.addObject(std::move(obj));
    }

    std::cout << "Scene loaded from " << filepath << " (" << scene.objects.size() << " objects)\n";
    return true;
}
