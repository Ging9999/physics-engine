#pragma once
#include <glad/glad.h>
#include <vector>
#include "math/Vec3.h"

struct Vertex {
    Vec3  position;
    Vec3  normal;
    float u = 0.0f;
    float v = 0.0f;
};

class Mesh {
public:
    Mesh() = default;
    ~Mesh();

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void create(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    void draw() const;

    static Mesh createCube();
    static Mesh createPlane(float size);
    static Mesh createSphere(int stacks, int slices, float radius);

    unsigned int getIndexCount() const { return m_indexCount; }

private:
    unsigned int m_VAO       = 0;
    unsigned int m_VBO       = 0;
    unsigned int m_EBO       = 0;
    unsigned int m_indexCount = 0;
};
