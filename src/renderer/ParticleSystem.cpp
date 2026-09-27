#include "renderer/ParticleSystem.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

ParticleSystem::~ParticleSystem() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

void ParticleSystem::init() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // layout: [x,y,z, r,g,b, a] - 7 floats per particle
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 7 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // alpha
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void ParticleSystem::emit(Vec3 position, Vec3 baseColor, int count) {
    for (int i = 0; i < count; i++) {
        if ((int)m_particles.size() >= MAX_PARTICLES) break;

        auto frand = []() { return (float)rand() / (float)RAND_MAX; };

        Vec3 dir = {frand() - 0.5f, frand() * 0.5f + 0.2f, frand() - 0.5f};
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        if (len > 0.0f) dir = dir * (1.0f / len);
        float speed = 2.0f + frand() * 4.0f;

        Particle p;
        p.position = position;
        p.velocity = dir * speed;
        p.color    = {baseColor.x + (frand()-0.5f)*0.3f,
                      baseColor.y + (frand()-0.5f)*0.3f,
                      baseColor.z + (frand()-0.5f)*0.3f};
        p.color.x = std::max(0.0f, std::min(1.0f, p.color.x));
        p.color.y = std::max(0.0f, std::min(1.0f, p.color.y));
        p.color.z = std::max(0.0f, std::min(1.0f, p.color.z));
        p.life    = 0.5f + frand() * 0.7f;
        p.maxLife = p.life;
        m_particles.push_back(p);
    }
}

void ParticleSystem::update(float dt) {
    for (int i = (int)m_particles.size() - 1; i >= 0; i--) {
        auto& p = m_particles[i];
        p.velocity.y -= 9.81f * dt;
        p.position   += p.velocity * dt;
        p.life       -= dt;
        if (p.life <= 0.0f) {
            m_particles[i] = m_particles.back();
            m_particles.pop_back();
        }
    }
}

void ParticleSystem::draw(Shader& shader, const Mat4& view, const Mat4& projection) {
    if (m_particles.empty()) return;

    // Build vertex buffer
    std::vector<float> data;
    data.reserve(m_particles.size() * 7);
    for (auto& p : m_particles) {
        float alpha = p.life / p.maxLife;
        data.push_back(p.position.x); data.push_back(p.position.y); data.push_back(p.position.z);
        data.push_back(p.color.x);    data.push_back(p.color.y);    data.push_back(p.color.z);
        data.push_back(alpha);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, data.size() * sizeof(float), data.data());

    shader.use();
    shader.setMat4("view",       view);
    shader.setMat4("projection", projection);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, (GLsizei)m_particles.size());
    glBindVertexArray(0);

    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_BLEND);
}
