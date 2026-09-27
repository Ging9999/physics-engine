#pragma once
#include "math/Vec3.h"
#include <vector>

class SceneObject;

struct CollisionEvent {
    Vec3  position;
    Vec3  colorA;
    Vec3  colorB;
    float intensity;
};

class PhysicsWorld {
public:
    Vec3 gravity            = {0.0f, -9.81f, 0.0f};
    bool paused             = false;
    int  grabbedObjectIndex = -1;

    std::vector<CollisionEvent> collisionEvents;

    void step(float dt, std::vector<SceneObject>& objects);

private:
    void applyGravity(std::vector<SceneObject>& objects);
    void integrateAll(float dt, std::vector<SceneObject>& objects);
    void detectAndResolveCollisions(std::vector<SceneObject>& objects);
    void clearAllForces(std::vector<SceneObject>& objects);
};
