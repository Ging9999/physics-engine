#pragma once
#include <glad/glad.h>
#include "math/Vec3.h"
#include "math/Mat4.h"
#include "renderer/Shader.h"
#include "renderer/Mesh.h"

class DebugRenderer {
public:
    bool visible = true;

    DebugRenderer() = default;
    ~DebugRenderer();

    DebugRenderer(const DebugRenderer&) = delete;
    DebugRenderer& operator=(const DebugRenderer&) = delete;

    void init();
    void setRay(Vec3 start, Vec3 end, bool hit, Vec3 hitPoint);
    void draw(Shader& flatShader, const Mat4& view, const Mat4& projection, Mesh& sphereMesh);

private:
    void updateLineBuffer(Vec3 start, Vec3 end);

    unsigned int m_lineVAO = 0;
    unsigned int m_lineVBO = 0;

    Vec3 m_lineStart;
    Vec3 m_lineEnd;
    Vec3 m_lineColor;
    Vec3 m_hitPoint;
    bool m_hasRay = false;
    bool m_hasHit = false;
};
