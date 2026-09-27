#pragma once
#include "math/Vec3.h"

class RigidBody {
public:
    Vec3  position;
    Vec3  velocity;
    Vec3  acceleration;
    Vec3  forceAccumulator;

    Vec3  rotation;             // Euler angles (radians)
    Vec3  angularVelocity;
    Vec3  torqueAccumulator;
    float inertiaTensorScalar = 1.0f;

    float mass        = 1.0f;
    float restitution = 0.5f;
    bool  isStatic    = false;
    float sleepTimer  = 0.0f;
    bool  sleeping    = false;

    void  applyForce(const Vec3& force);
    void  applyTorque(const Vec3& torque);
    void  integrate(float dt);
    void  clearForces();
    float inverseMass() const;
};
