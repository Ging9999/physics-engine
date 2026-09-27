#include "physics/Collider.h"

Collider Collider::createSphere(float radius) {
    Collider c;
    c.type          = ColliderType::SPHERE;
    c.sphere.radius = radius;
    return c;
}

Collider Collider::createAABB(Vec3 halfExtents) {
    Collider c;
    c.type             = ColliderType::AABB;
    c.aabb.halfExtents = halfExtents;
    return c;
}
