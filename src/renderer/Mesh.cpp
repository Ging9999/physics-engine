#include "renderer/Mesh.h"
#include <cmath>

Mesh::~Mesh() {
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
}

Mesh::Mesh(Mesh&& other) noexcept
    : m_VAO(other.m_VAO), m_VBO(other.m_VBO), m_EBO(other.m_EBO), m_indexCount(other.m_indexCount) {
    other.m_VAO        = 0;
    other.m_VBO        = 0;
    other.m_EBO        = 0;
    other.m_indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO) glDeleteBuffers(1, &m_VBO);
        if (m_EBO) glDeleteBuffers(1, &m_EBO);
        m_VAO        = other.m_VAO;
        m_VBO        = other.m_VBO;
        m_EBO        = other.m_EBO;
        m_indexCount = other.m_indexCount;
        other.m_VAO        = 0;
        other.m_VBO        = 0;
        other.m_EBO        = 0;
        other.m_indexCount = 0;
    }
    return *this;
}

void Mesh::create(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    m_indexCount = static_cast<unsigned int>(indices.size());

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::draw() const {
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Mesh Mesh::createCube() {
    // Each face: 4 vertices with position, normal, UV (0,0)(1,0)(1,1)(0,1)
    std::vector<Vertex> vertices = {
        // Front  (z = +0.5)
        {{-0.5f,-0.5f, 0.5f},{0,0,1},0,0}, {{ 0.5f,-0.5f, 0.5f},{0,0,1},1,0},
        {{ 0.5f, 0.5f, 0.5f},{0,0,1},1,1}, {{-0.5f, 0.5f, 0.5f},{0,0,1},0,1},
        // Back   (z = -0.5)
        {{ 0.5f,-0.5f,-0.5f},{0,0,-1},0,0}, {{-0.5f,-0.5f,-0.5f},{0,0,-1},1,0},
        {{-0.5f, 0.5f,-0.5f},{0,0,-1},1,1}, {{ 0.5f, 0.5f,-0.5f},{0,0,-1},0,1},
        // Top    (y = +0.5)
        {{-0.5f, 0.5f, 0.5f},{0,1,0},0,0}, {{ 0.5f, 0.5f, 0.5f},{0,1,0},1,0},
        {{ 0.5f, 0.5f,-0.5f},{0,1,0},1,1}, {{-0.5f, 0.5f,-0.5f},{0,1,0},0,1},
        // Bottom (y = -0.5)
        {{-0.5f,-0.5f,-0.5f},{0,-1,0},0,0}, {{ 0.5f,-0.5f,-0.5f},{0,-1,0},1,0},
        {{ 0.5f,-0.5f, 0.5f},{0,-1,0},1,1}, {{-0.5f,-0.5f, 0.5f},{0,-1,0},0,1},
        // Right  (x = +0.5)
        {{ 0.5f,-0.5f, 0.5f},{1,0,0},0,0}, {{ 0.5f,-0.5f,-0.5f},{1,0,0},1,0},
        {{ 0.5f, 0.5f,-0.5f},{1,0,0},1,1}, {{ 0.5f, 0.5f, 0.5f},{1,0,0},0,1},
        // Left   (x = -0.5)
        {{-0.5f,-0.5f,-0.5f},{-1,0,0},0,0}, {{-0.5f,-0.5f, 0.5f},{-1,0,0},1,0},
        {{-0.5f, 0.5f, 0.5f},{-1,0,0},1,1}, {{-0.5f, 0.5f,-0.5f},{-1,0,0},0,1},
    };

    std::vector<unsigned int> indices;
    for (unsigned int face = 0; face < 6; face++) {
        unsigned int b = face * 4;
        indices.insert(indices.end(), {b,b+1,b+2, b,b+2,b+3});
    }

    Mesh mesh;
    mesh.create(vertices, indices);
    return mesh;
}

Mesh Mesh::createPlane(float size) {
    float h = size / 2.0f;
    float t = 5.0f; // tiling factor
    std::vector<Vertex> vertices = {
        {{-h,0,-h},{0,1,0},0,0},   {{ h,0,-h},{0,1,0},t,0},
        {{ h,0, h},{0,1,0},t,t},   {{-h,0, h},{0,1,0},0,t},
    };
    std::vector<unsigned int> indices = {0,1,2, 0,2,3};

    Mesh mesh;
    mesh.create(vertices, indices);
    return mesh;
}

Mesh Mesh::createSphere(int stacks, int slices, float radius) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    const float PI = 3.14159265359f;

    for (int i = 0; i <= stacks; i++) {
        float phi = PI * float(i) / float(stacks);
        for (int j = 0; j <= slices; j++) {
            float theta = 2.0f * PI * float(j) / float(slices);
            float x = sinf(phi) * cosf(theta);
            float y = cosf(phi);
            float z = sinf(phi) * sinf(theta);
            Vertex v;
            v.position = {x * radius, y * radius, z * radius};
            v.normal   = {x, y, z};
            v.u        = float(j) / float(slices);
            v.v        = float(i) / float(stacks);
            vertices.push_back(v);
        }
    }

    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < slices; j++) {
            unsigned int a = i * (slices + 1) + j;
            unsigned int b = a + slices + 1;
            indices.insert(indices.end(), {a, b, a+1, a+1, b, b+1});
        }
    }

    Mesh mesh;
    mesh.create(vertices, indices);
    return mesh;
}
