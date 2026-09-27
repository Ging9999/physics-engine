#pragma once
#include <string>
#include "renderer/Mesh.h"

class ObjLoader {
public:
    static bool load(const std::string& filepath, Mesh& outMesh);

private:
    struct FaceIndex {
        int vertex   = -1;
        int texcoord = -1;
        int normal   = -1;
    };
    static FaceIndex parseFaceVertex(const std::string& token);
};
