#pragma once
#include "math/Vec3.h"

enum class ColliderType { SPHERE, AABB };

struct SphereCollider {
    float radius = 0.5f;
};

struct AABBCollider {
    Vec3 halfExtents = {0.5f, 0.5f, 0.5f};
};

struct Collider {
    ColliderType  type;
    SphereCollider sphere;
    AABBCollider   aabb;

    static Collider createSphere(float radius);
    static Collider createAABB(Vec3 halfExtents);
};
