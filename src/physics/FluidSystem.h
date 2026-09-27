#pragma once
#include <vector>
#include "math/Vec3.h"
#include "physics/FluidParticle.h"
#include "physics/SpatialHashGrid.h"

class SceneObject;

class FluidSystem {
public:
    static constexpr int MAX_PARTICLES = 500;

    FluidSystem();

    void spawnBlock(Vec3 center, int countPerAxis = 5, float spacing = 0.3f);
    void spawnStream(Vec3 position, Vec3 velocity, int count = 5);
    void step(float dt, std::vector<SceneObject>& sceneObjects);
    void clear();

    const std::vector<FluidParticle>& getParticles() const { return m_particles; }
    int getParticleCount() const;

private:
    std::vector<FluidParticle> m_particles;
    SpatialHashGrid m_grid;

    float m_smoothingRadius = 1.0f;
    float m_particleMass    = 1.0f;
    float m_restDensity     = 1000.0f;
    float m_gasConstant     = 200.0f;
    float m_viscosity       = 50.0f;
    float m_damping         = 0.98f;
    float m_particleRadius  = 0.15f;
    float m_bounceRestitution = 0.3f;

    void buildGrid();
    void computeDensityAndPressure();
    void computeForces();
    void integrate(float dt);
    void handleBoundaries();
    void handleSceneCollisions(std::vector<SceneObject>& sceneObjects);

    float poly6(float r, float h) const;
    Vec3  spikyGrad(Vec3 rVec, float r, float h) const;
    float viscLaplacian(float r, float h) const;
};
