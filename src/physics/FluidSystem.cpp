#include "physics/FluidSystem.h"
#include "scene/SceneObject.h"
#include "physics/Collider.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

static const float PI       = 3.14159265f;
static const float GRAVITY_Y = -9.81f;

FluidSystem::FluidSystem() : m_grid(1.0f) {}

// Kernel functions

float FluidSystem::poly6(float r, float h) const {
    if (r >= h) return 0.0f;
    float h2   = h * h;
    float r2   = r * r;
    float diff = h2 - r2;
    return 315.0f / (64.0f * PI * std::pow(h, 9.0f)) * diff * diff * diff;
}

Vec3 FluidSystem::spikyGrad(Vec3 rVec, float r, float h) const {
    if (r >= h || r < 1e-6f) return {0, 0, 0};
    float coeff = -45.0f / (PI * std::pow(h, 6.0f)) * (h - r) * (h - r);
    return rVec * (coeff / r);
}

float FluidSystem::viscLaplacian(float r, float h) const {
    if (r >= h) return 0.0f;
    return 45.0f / (PI * std::pow(h, 6.0f)) * (h - r);
}

// SPH pipeline

void FluidSystem::buildGrid() {
    m_grid.clear();
    for (int i = 0; i < (int)m_particles.size(); i++) {
        if (m_particles[i].active)
            m_grid.insert(i, m_particles[i].position);
    }
}

void FluidSystem::computeDensityAndPressure() {
    for (auto& p : m_particles) {
        if (!p.active) continue;
        p.density = 0.0f;

        auto neighbours = m_grid.getNeighbours(p.position);
        for (int j : neighbours) {
            auto& pj = m_particles[j];
            if (!pj.active) continue;
            float r = (p.position - pj.position).length();
            p.density += m_particleMass * poly6(r, m_smoothingRadius);
        }

        if (p.density < m_restDensity * 0.01f)
            p.density = m_restDensity * 0.01f;

        p.pressure = m_gasConstant * (p.density - m_restDensity);
    }
}

void FluidSystem::computeForces() {
    for (int i = 0; i < (int)m_particles.size(); i++) {
        auto& pi = m_particles[i];
        if (!pi.active) continue;

        Vec3 pressureForce = {0, 0, 0};
        Vec3 viscosityForce = {0, 0, 0};

        auto neighbours = m_grid.getNeighbours(pi.position);
        for (int j : neighbours) {
            if (i == j) continue;
            auto& pj = m_particles[j];
            if (!pj.active) continue;

            Vec3  rVec = pi.position - pj.position;
            float r    = rVec.length();

            if (r < m_smoothingRadius && r > 1e-6f) {
                Vec3 gradW = spikyGrad(rVec, r, m_smoothingRadius);
                pressureForce -= gradW * (m_particleMass *
                    (pi.pressure + pj.pressure) / (2.0f * pj.density));

                float lapW = viscLaplacian(r, m_smoothingRadius);
                viscosityForce += (pj.velocity - pi.velocity) *
                    (m_viscosity * m_particleMass * lapW / pj.density);
            }
        }

        Vec3 gravityForce = {0.0f, GRAVITY_Y * pi.density, 0.0f};
        pi.force = pressureForce + viscosityForce + gravityForce;
    }
}

void FluidSystem::integrate(float dt) {
    const float maxSpeed = 30.0f;
    for (auto& p : m_particles) {
        if (!p.active) continue;

        Vec3 accel = p.force * (1.0f / p.density);
        p.velocity += accel * dt;
        p.velocity *= m_damping;

        if (p.velocity.lengthSquared() > maxSpeed * maxSpeed)
            p.velocity = p.velocity.normalized() * maxSpeed;

        p.position += p.velocity * dt;
    }
}

void FluidSystem::handleBoundaries() {
    const float groundY  = 0.0f;
    const float boundsXZ = 15.0f;
    const float boundsY  = 30.0f;

    for (auto& p : m_particles) {
        if (!p.active) continue;

        if (p.position.y < groundY + m_particleRadius) {
            p.position.y  = groundY + m_particleRadius;
            p.velocity.y *= -m_bounceRestitution;
            p.velocity.x *= 0.95f;
            p.velocity.z *= 0.95f;
        }
        if (p.position.y > boundsY) {
            p.position.y  = boundsY;
            p.velocity.y *= -m_bounceRestitution;
        }
        if (p.position.x < -boundsXZ + m_particleRadius) {
            p.position.x  = -boundsXZ + m_particleRadius;
            p.velocity.x *= -m_bounceRestitution;
        }
        if (p.position.x > boundsXZ - m_particleRadius) {
            p.position.x  = boundsXZ - m_particleRadius;
            p.velocity.x *= -m_bounceRestitution;
        }
        if (p.position.z < -boundsXZ + m_particleRadius) {
            p.position.z  = -boundsXZ + m_particleRadius;
            p.velocity.z *= -m_bounceRestitution;
        }
        if (p.position.z > boundsXZ - m_particleRadius) {
            p.position.z  = boundsXZ - m_particleRadius;
            p.velocity.z *= -m_bounceRestitution;
        }
        if (p.position.y < -50.0f) p.active = false;
    }
}

void FluidSystem::handleSceneCollisions(std::vector<SceneObject>& sceneObjects) {
    for (auto& p : m_particles) {
        if (!p.active) continue;

        for (auto& obj : sceneObjects) {
            Vec3 objPos = obj.rigidBody.position;

            if (obj.collider.type == ColliderType::SPHERE) {
                Vec3  diff    = p.position - objPos;
                float dist    = diff.length();
                float minDist = obj.collider.sphere.radius + m_particleRadius;

                if (dist < minDist && dist > 1e-6f) {
                    Vec3  normal = diff * (1.0f / dist);
                    p.position   = objPos + normal * minDist;
                    float vDotN  = p.velocity.dot(normal);
                    if (vDotN < 0.0f)
                        p.velocity -= normal * ((1.0f + m_bounceRestitution) * vDotN);

                    if (!obj.rigidBody.isStatic)
                        obj.rigidBody.applyForce(normal * (-vDotN * m_particleMass * 2.0f));
                }
            }
            else if (obj.collider.type == ColliderType::AABB) {
                Vec3 halfExt = obj.collider.aabb.halfExtents;
                halfExt.x *= obj.scale.x;
                halfExt.y *= obj.scale.y;
                halfExt.z *= obj.scale.z;

                Vec3 local = p.position - objPos;
                Vec3 closest = {
                    std::max(-halfExt.x, std::min(local.x, halfExt.x)),
                    std::max(-halfExt.y, std::min(local.y, halfExt.y)),
                    std::max(-halfExt.z, std::min(local.z, halfExt.z))
                };

                Vec3  diff = local - closest;
                float dist = diff.length();

                if (dist < m_particleRadius && dist > 1e-6f) {
                    Vec3  normal = diff * (1.0f / dist);
                    p.position   = objPos + closest + normal * m_particleRadius;
                    float vDotN  = p.velocity.dot(normal);
                    if (vDotN < 0.0f)
                        p.velocity -= normal * ((1.0f + m_bounceRestitution) * vDotN);

                    if (!obj.rigidBody.isStatic)
                        obj.rigidBody.applyForce(normal * (-vDotN * m_particleMass * 2.0f));
                }
            }
        }
    }
}

// Public interface

void FluidSystem::step(float dt, std::vector<SceneObject>& sceneObjects) {
    if (m_particles.empty()) return;
    buildGrid();
    computeDensityAndPressure();
    computeForces();
    integrate(dt);
    handleBoundaries();
    handleSceneCollisions(sceneObjects);
}

void FluidSystem::spawnBlock(Vec3 center, int countPerAxis, float spacing) {
    float offset = (countPerAxis - 1) * spacing * 0.5f;
    for (int x = 0; x < countPerAxis; x++) {
        for (int y = 0; y < countPerAxis; y++) {
            for (int z = 0; z < countPerAxis; z++) {
                if (getParticleCount() >= MAX_PARTICLES) return;
                FluidParticle p;
                p.position = center + Vec3{
                    x * spacing - offset,
                    y * spacing - offset,
                    z * spacing - offset
                };
                p.active = true;
                m_particles.push_back(p);
            }
        }
    }
}

void FluidSystem::spawnStream(Vec3 position, Vec3 velocity, int count) {
    for (int i = 0; i < count; i++) {
        if (getParticleCount() >= MAX_PARTICLES) return;
        FluidParticle p;
        p.position = position + Vec3{
            ((rand() % 100) / 100.0f - 0.5f) * 0.2f,
            ((rand() % 100) / 100.0f - 0.5f) * 0.2f,
            ((rand() % 100) / 100.0f - 0.5f) * 0.2f
        };
        p.velocity = velocity;
        p.active   = true;
        m_particles.push_back(p);
    }
}

int FluidSystem::getParticleCount() const {
    int n = 0;
    for (const auto& p : m_particles)
        if (p.active) n++;
    return n;
}

void FluidSystem::clear() {
    m_particles.clear();
}
