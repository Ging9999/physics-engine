#include "renderer/ObjLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

ObjLoader::FaceIndex ObjLoader::parseFaceVertex(const std::string& token) {
    FaceIndex fi;
    size_t first = token.find('/');
    if (first == std::string::npos) {
        fi.vertex = std::stoi(token) - 1;
        return fi;
    }
    fi.vertex = std::stoi(token.substr(0, first)) - 1;
    size_t second = token.find('/', first + 1);
    if (second == std::string::npos) {
        fi.texcoord = std::stoi(token.substr(first + 1)) - 1;
        return fi;
    }
    if (second != first + 1)
        fi.texcoord = std::stoi(token.substr(first + 1, second - first - 1)) - 1;
    if (second + 1 < token.size())
        fi.normal = std::stoi(token.substr(second + 1)) - 1;
    return fi;
}

bool ObjLoader::load(const std::string& filepath, Mesh& outMesh) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "ObjLoader: cannot open " << filepath << "\n";
        return false;
    }

    struct Vec2 { float x, y; };

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<Vec2> texcoords;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::map<std::string, unsigned int> vertexMap;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string tok;
        iss >> tok;

        if (tok == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            positions.push_back({x, y, z});
        } else if (tok == "vn") {
            float x, y, z;
            iss >> x >> y >> z;
            normals.push_back({x, y, z});
        } else if (tok == "vt") {
            float u, v;
            iss >> u >> v;
            texcoords.push_back({u, v});
        } else if (tok == "f") {
            std::vector<FaceIndex> fv;
            std::string s;
            while (iss >> s) fv.push_back(parseFaceVertex(s));

            for (size_t i = 1; i + 1 < fv.size(); i++) {
                FaceIndex tri[3] = {fv[0], fv[i], fv[i+1]};
                for (auto& fi : tri) {
                    std::string key = std::to_string(fi.vertex) + "/"
                                    + std::to_string(fi.texcoord) + "/"
                                    + std::to_string(fi.normal);
                    auto it = vertexMap.find(key);
                    if (it != vertexMap.end()) {
                        indices.push_back(it->second);
                    } else {
                        Vertex v;
                        v.position = (fi.vertex >= 0 && fi.vertex < (int)positions.size())
                                     ? positions[fi.vertex] : Vec3{};
                        v.normal   = (fi.normal >= 0 && fi.normal < (int)normals.size())
                                     ? normals[fi.normal] : Vec3{0, 1, 0};
                        if (fi.texcoord >= 0 && fi.texcoord < (int)texcoords.size()) {
                            v.u = texcoords[fi.texcoord].x;
                            v.v = texcoords[fi.texcoord].y;
                        }
                        unsigned int idx = static_cast<unsigned int>(vertices.size());
                        vertices.push_back(v);
                        vertexMap[key] = idx;
                        indices.push_back(idx);
                    }
                }
            }
        }
    }

    if (vertices.empty()) {
        std::cerr << "ObjLoader: no geometry in " << filepath << "\n";
        return false;
    }

    outMesh.create(vertices, indices);
    std::cout << "ObjLoader: loaded " << filepath
              << " (" << vertices.size() << " verts, " << indices.size()/3 << " tris)\n";
    return true;
}
