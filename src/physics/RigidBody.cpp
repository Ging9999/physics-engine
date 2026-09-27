#include "physics/RigidBody.h"
#include <cmath>
#include <algorithm>

static constexpr float kSleepSpeedSq   = 0.05f;
static constexpr float kSleepDelay     = 0.5f;
static constexpr float kMaxSpeed       = 50.0f;
static constexpr float kMaxAngular     = 20.0f;

void RigidBody::applyForce(const Vec3& force) {
    if (isStatic) return;
    sleeping   = false;
    sleepTimer = 0.0f;
    forceAccumulator += force;
}

void RigidBody::applyTorque(const Vec3& torque) {
    if (isStatic) return;
    torqueAccumulator += torque;
}

void RigidBody::integrate(float dt) {
    if (isStatic || sleeping) return;

    float invMass = inverseMass();

    // Linear integration
    acceleration = forceAccumulator * invMass;
    velocity    += acceleration * dt;

    float speedSq = velocity.lengthSquared();
    if (speedSq > kMaxSpeed * kMaxSpeed) {
        float s = kMaxSpeed / std::sqrt(speedSq);
        velocity = velocity * s;
    }

    position += velocity * dt;

    // Angular integration
    Vec3 angAccel = torqueAccumulator * (1.0f / inertiaTensorScalar);
    angularVelocity += angAccel * dt;
    angularVelocity  = angularVelocity * 0.98f;  // damping

    float angSq = angularVelocity.lengthSquared();
    if (angSq > kMaxAngular * kMaxAngular) {
        float s = kMaxAngular / std::sqrt(angSq);
        angularVelocity = angularVelocity * s;
    }

    rotation += angularVelocity * dt;

    // Sleep check
    if (velocity.lengthSquared() < kSleepSpeedSq &&
        angularVelocity.lengthSquared() < kSleepSpeedSq) {
        sleepTimer += dt;
        if (sleepTimer >= kSleepDelay) {
            sleeping        = true;
            velocity        = Vec3{};
            angularVelocity = Vec3{};
        }
    } else {
        sleepTimer = 0.0f;
    }
}

void RigidBody::clearForces() {
    forceAccumulator  = Vec3{};
    torqueAccumulator = Vec3{};
}

float RigidBody::inverseMass() const {
    if (isStatic || mass <= 0.0f) return 0.0f;
    return 1.0f / mass;
}
