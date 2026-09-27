#include "renderer/HeightFieldWater.h"
#include "scene/SceneObject.h"
#include "physics/Collider.h"
#include <cmath>
#include <algorithm>

HeightFieldWater::HeightFieldWater(int cols, int rows,
                                   float width, float depth, float restH)
    : m_cols(cols), m_rows(rows),
      m_width(width), m_depth(depth),
      m_restH(restH)
{
    m_cellW   = width  / (cols - 1);
    m_cellD   = depth  / (rows - 1);
    m_originX = -width  * 0.5f;
    m_originZ = -depth  * 0.5f;

    m_height.assign(cols * rows, 0.0f);
    m_vel   .assign(cols * rows, 0.0f);
}

HeightFieldWater::~HeightFieldWater() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); }
    if (m_ebo) { glDeleteBuffers(1, &m_ebo); }
}

// init

void HeightFieldWater::init() {
    buildGeometry();
}

// Simulation

void HeightFieldWater::step(float /*dt*/) {
    // Wave equation (interior cells only, boundaries are absorbing)
    for (int z = 1; z < m_rows - 1; z++) {
        for (int x = 1; x < m_cols - 1; x++) {
            float h   = m_height[idx(x,   z)];
            float avg = (m_height[idx(x-1, z)] + m_height[idx(x+1, z)] +
                         m_height[idx(x,   z-1)] + m_height[idx(x,   z+1)]) * 0.25f;
            m_vel[idx(x,z)] += (avg - h) * m_waveSpeed;
        }
    }
    for (int i = 0; i < m_cols * m_rows; i++) {
        m_height[i] += m_vel[i];
        m_vel[i]    *= m_damping;
    }
    // Absorbing boundaries: clamp edges toward zero
    for (int x = 0; x < m_cols; x++) {
        m_height[idx(x, 0)]         *= 0.5f;
        m_height[idx(x, m_rows-1)]  *= 0.5f;
        m_vel   [idx(x, 0)]          = 0.0f;
        m_vel   [idx(x, m_rows-1)]   = 0.0f;
    }
    for (int z = 0; z < m_rows; z++) {
        m_height[idx(0,        z)] *= 0.5f;
        m_height[idx(m_cols-1, z)] *= 0.5f;
        m_vel   [idx(0,        z)]  = 0.0f;
        m_vel   [idx(m_cols-1, z)]  = 0.0f;
    }
}

// Object splash

void HeightFieldWater::handleObjectSplash(const std::vector<SceneObject>& objects) {
    for (const auto& obj : objects) {
        if (obj.rigidBody.isStatic) continue;
        float vy = obj.rigidBody.velocity.y;
        if (vy > -1.5f) continue;  // only significant downward motion

        Vec3  pos = obj.rigidBody.position;
        float objRadius, objBottom;
        if (obj.collider.type == ColliderType::AABB) {
            objRadius = (obj.collider.aabb.halfExtents.x + obj.collider.aabb.halfExtents.z) * 0.5f;
            objBottom = pos.y - obj.collider.aabb.halfExtents.y;
        } else {
            objRadius = obj.collider.sphere.radius;
            objBottom = pos.y - obj.collider.sphere.radius;
        }

        // Check if bottom has reached the water surface
        if (objBottom <= m_restH + 0.3f) {
            float strength = std::min(-vy * 0.18f, 1.2f);
            disturb(pos.x, pos.z, strength, objRadius + 0.8f);
        }
    }
}

// Disturb surface

void HeightFieldWater::disturb(float worldX, float worldZ, float amount, float radius) {
    int cx = (int)((worldX - m_originX) / m_cellW);
    int cz = (int)((worldZ - m_originZ) / m_cellD);
    int r  = (int)(radius / std::min(m_cellW, m_cellD)) + 1;

    for (int dz = -r; dz <= r; dz++) {
        for (int dx = -r; dx <= r; dx++) {
            int nx = cx + dx, nz = cz + dz;
            if (nx < 1 || nx >= m_cols-1 || nz < 1 || nz >= m_rows-1) continue;
            float dist    = std::sqrt((float)(dx*dx + dz*dz)) * std::min(m_cellW, m_cellD);
            float falloff = std::max(0.0f, 1.0f - dist / radius);
            m_height[idx(nx, nz)] -= amount * falloff;
        }
    }
}

// getHeight

float HeightFieldWater::getHeight(float worldX, float worldZ) const {
    int cx = std::max(0, std::min(m_cols - 1, (int)((worldX - m_originX) / m_cellW)));
    int cz = std::max(0, std::min(m_rows - 1, (int)((worldZ - m_originZ) / m_cellD)));
    return m_restH + m_height[idx(cx, cz)];
}

// applyBuoyancy

void HeightFieldWater::applyBuoyancy(std::vector<SceneObject>& objects) {
    const float waterDensity = 3.0f;
    const float g            = 9.81f;
    const float dragCoeff    = 3.0f;

    for (auto& obj : objects) {
        if (obj.rigidBody.isStatic) continue;

        Vec3  pos  = obj.rigidBody.position;
        float surf = getHeight(pos.x, pos.z);

        float subFrac = 0.0f;
        float volume  = 0.0f;

        if (obj.collider.type == ColliderType::AABB) {
            Vec3  he  = obj.collider.aabb.halfExtents;
            float bot = pos.y - he.y;
            float top = pos.y + he.y;
            if (bot < surf) {
                float sub = std::min(surf, top) - std::max(bot, 0.0f);
                subFrac = std::max(0.0f, std::min(1.0f, sub / (he.y * 2.0f)));
                volume  = 8.0f * he.x * he.y * he.z;
            }
        } else {
            float r   = obj.collider.sphere.radius;
            float bot = pos.y - r;
            float top = pos.y + r;
            if (bot < surf) {
                float sub = std::min(surf, top) - std::max(bot, 0.0f);
                subFrac = std::max(0.0f, std::min(1.0f, sub / (2.0f * r)));
                volume  = (4.0f / 3.0f) * 3.14159265f * r * r * r;
            }
        }

        if (subFrac > 0.0f) {
            obj.rigidBody.applyForce({0.0f, waterDensity * g * volume * subFrac, 0.0f});
            Vec3  vel  = obj.rigidBody.velocity;
            float drag = dragCoeff * subFrac;
            obj.rigidBody.applyForce({-vel.x * drag, -vel.y * drag, -vel.z * drag});
        }
    }
}

// Mesh

void HeightFieldWater::buildGeometry() {
    int N = m_cols * m_rows;
    m_verts.resize(N * 8);

    // Build flat initial vertex buffer
    for (int z = 0; z < m_rows; z++) {
        for (int x = 0; x < m_cols; x++) {
            int vi = (z * m_cols + x) * 8;
            m_verts[vi+0] = m_originX + x * m_cellW;
            m_verts[vi+1] = m_restH;
            m_verts[vi+2] = m_originZ + z * m_cellD;
            m_verts[vi+3] = 0.0f; m_verts[vi+4] = 1.0f; m_verts[vi+5] = 0.0f;  // up normal
            m_verts[vi+6] = (float)x / (m_cols - 1);
            m_verts[vi+7] = (float)z / (m_rows - 1);
        }
    }

    // Build indices: two triangles per quad
    for (int z = 0; z < m_rows - 1; z++) {
        for (int x = 0; x < m_cols - 1; x++) {
            unsigned int tl = (unsigned int)(z * m_cols + x);
            unsigned int tr = tl + 1u;
            unsigned int bl = tl + (unsigned int)m_cols;
            unsigned int br = bl + 1u;
            m_indices.push_back(tl); m_indices.push_back(bl); m_indices.push_back(tr);
            m_indices.push_back(tr); m_indices.push_back(bl); m_indices.push_back(br);
        }
    }
    m_indexCount = (int)m_indices.size();

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(m_verts.size() * sizeof(float)),
                 m_verts.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(m_indices.size() * sizeof(unsigned int)),
                 m_indices.data(), GL_STATIC_DRAW);

    const int stride = 8 * (int)sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void HeightFieldWater::updateVertices() {
    for (int z = 0; z < m_rows; z++) {
        for (int x = 0; x < m_cols; x++) {
            int   vi   = (z * m_cols + x) * 8;
            float h    = m_restH + m_height[idx(x,z)];
            m_verts[vi+1] = h;

            // Central-difference surface normal: (-dh/dx, 1, -dh/dz) normalised
            float hl = m_restH + m_height[idx(x > 0       ? x-1 : x,   z)];
            float hr = m_restH + m_height[idx(x < m_cols-1 ? x+1 : x,   z)];
            float hu = m_restH + m_height[idx(x,   z > 0       ? z-1 : z)];
            float hd = m_restH + m_height[idx(x,   z < m_rows-1 ? z+1 : z)];

            float dhdx = (hr - hl) / (2.0f * m_cellW);
            float dhdz = (hd - hu) / (2.0f * m_cellD);

            float nx = -dhdx, ny = 1.0f, nz = -dhdz;
            float len = std::sqrt(nx*nx + ny*ny + nz*nz);
            if (len > 1e-6f) { nx /= len; ny /= len; nz /= len; }
            m_verts[vi+3] = nx;
            m_verts[vi+4] = ny;
            m_verts[vi+5] = nz;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (GLsizeiptr)(m_verts.size() * sizeof(float)), m_verts.data());
}

// draw

void HeightFieldWater::draw(Shader& waterShader, const Mat4& view, const Mat4& projection,
                             const Vec3& lightPos, const Vec3& viewPos, float time) {
    if (!m_vao) return;

    updateVertices();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    waterShader.use();
    waterShader.setMat4 ("view",       view);
    waterShader.setMat4 ("projection", projection);
    waterShader.setVec3 ("lightPos",   lightPos);
    waterShader.setVec3 ("viewPos",    viewPos);
    waterShader.setFloat("time",       time);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
}
