#pragma once
#include <glad/glad.h>
#include <vector>
#include "math/Vec3.h"
#include "math/Mat4.h"
#include "renderer/Shader.h"

struct Particle {
    Vec3  position;
    Vec3  velocity;
    Vec3  color;
    float life;
    float maxLife;
};

class ParticleSystem {
public:
    static constexpr int MAX_PARTICLES = 500;

    ~ParticleSystem();
    void init();
    void emit(Vec3 position, Vec3 baseColor, int count = 15);
    void update(float dt);
    void draw(Shader& shader, const Mat4& view, const Mat4& projection);

private:
    std::vector<Particle> m_particles;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
};
