#pragma once
#include <glad/glad.h>
#include <vector>
#include "math/Vec3.h"
#include "math/Mat4.h"
#include "renderer/Shader.h"

class SceneObject;

class HeightFieldWater {
public:
    // cols/rows: grid resolution  width/depth: world-space size  restH: water level above y=0
    HeightFieldWater(int cols = 64, int rows = 64,
                     float width = 20.0f, float depth = 20.0f,
                     float restH = 0.3f);
    ~HeightFieldWater();

    HeightFieldWater(const HeightFieldWater&)            = delete;
    HeightFieldWater& operator=(const HeightFieldWater&) = delete;

    // Call once after GL context is ready
    void init();

    // Call every fixed physics step (60 Hz)
    void step(float dt);

    // Call every fixed step. Creates splashes when objects enter water.
    void handleObjectSplash(const std::vector<SceneObject>& objects);

    // Render. Call once per frame after opaque geometry.
    void draw(Shader& waterShader, const Mat4& view, const Mat4& projection,
              const Vec3& lightPos, const Vec3& viewPos, float time);

    // Push the surface down at a world (x,z) point to create a wave.
    void disturb(float worldX, float worldZ, float amount, float radius = 1.5f);

    // Returns the actual water surface height (restH + displacement) at a world XZ point
    float getHeight(float worldX, float worldZ) const;

    // Applies Archimedes buoyancy + drag to all non-static objects each fixed step
    void applyBuoyancy(std::vector<SceneObject>& objects);

private:
    int   m_cols, m_rows;
    float m_width, m_depth;
    float m_cellW, m_cellD;
    float m_originX, m_originZ;
    float m_restH;

    float m_waveSpeed = 0.20f;   // c: wave propagation speed (0 < c <= 0.5 for stability)
    float m_damping   = 0.993f;  // per-step energy loss

    std::vector<float> m_height;  // displacement from m_restH
    std::vector<float> m_vel;

    // GPU resources
    GLuint m_vao        = 0;
    GLuint m_vbo        = 0;
    GLuint m_ebo        = 0;
    int    m_indexCount = 0;

    std::vector<float>        m_verts;    // cpu-side vertex buffer [pos(3)+normal(3)+uv(2)]
    std::vector<unsigned int> m_indices;

    inline int idx(int x, int z) const { return z * m_cols + x; }

    void buildGeometry();   // create VAO/VBO/EBO and initial flat mesh
    void updateVertices();  // recompute positions + normals, upload via glBufferSubData
};
