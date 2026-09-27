#include "physics/CollisionDetection.h"
#include <cmath>
#include <algorithm>

CollisionInfo CollisionDetection::sphereVsSphere(const Vec3& posA, float rA,
                                                  const Vec3& posB, float rB) {
    CollisionInfo info;
    Vec3  delta    = posB - posA;
    float distance = delta.length();
    float sumR     = rA + rB;

    if (distance < sumR) {
        info.collided     = true;
        info.normal       = (distance < 1e-8f) ? Vec3{0,1,0} : delta.normalized();
        info.penetration  = sumR - distance;
    }
    return info;
}

CollisionInfo CollisionDetection::aabbVsAABB(const Vec3& posA, const Vec3& halfA,
                                              const Vec3& posB, const Vec3& halfB) {
    CollisionInfo info;
    Vec3  delta    = posB - posA;
    float overlapX = (halfA.x + halfB.x) - std::abs(delta.x);
    float overlapY = (halfA.y + halfB.y) - std::abs(delta.y);
    float overlapZ = (halfA.z + halfB.z) - std::abs(delta.z);

    if (overlapX <= 0 || overlapY <= 0 || overlapZ <= 0) return info;

    info.collided = true;
    if (overlapX <= overlapY && overlapX <= overlapZ) {
        info.penetration = overlapX;
        info.normal      = {delta.x < 0 ? -1.0f : 1.0f, 0, 0};
    } else if (overlapY <= overlapX && overlapY <= overlapZ) {
        info.penetration = overlapY;
        info.normal      = {0, delta.y < 0 ? -1.0f : 1.0f, 0};
    } else {
        info.penetration = overlapZ;
        info.normal      = {0, 0, delta.z < 0 ? -1.0f : 1.0f};
    }
    return info;
}

CollisionInfo CollisionDetection::sphereVsAABB(const Vec3& spherePos, float radius,
                                                const Vec3& boxPos, const Vec3& halfExtents) {
    CollisionInfo info;
    Vec3 closest;
    closest.x = std::max(boxPos.x - halfExtents.x, std::min(spherePos.x, boxPos.x + halfExtents.x));
    closest.y = std::max(boxPos.y - halfExtents.y, std::min(spherePos.y, boxPos.y + halfExtents.y));
    closest.z = std::max(boxPos.z - halfExtents.z, std::min(spherePos.z, boxPos.z + halfExtents.z));

    Vec3  delta  = spherePos - closest;
    float distSq = delta.dot(delta);

    if (distSq < radius * radius) {
        info.collided = true;
        float dist    = std::sqrt(distSq);
        info.normal      = (dist < 1e-8f) ? Vec3{0,1,0} : delta * (1.0f / dist);
        info.penetration = radius - dist;
    }
    return info;
}

CollisionInfo CollisionDetection::sphereVsGround(const Vec3& pos, float radius) {
    CollisionInfo info;
    float depth = pos.y - radius;
    if (depth < 0.0f) {
        info.collided    = true;
        info.normal      = {0, 1, 0};
        info.penetration = -depth;
    }
    return info;
}

CollisionInfo CollisionDetection::aabbVsGround(const Vec3& pos, const Vec3& halfExtents) {
    CollisionInfo info;
    float depth = pos.y - halfExtents.y;
    if (depth < 0.0f) {
        info.collided    = true;
        info.normal      = {0, 1, 0};
        info.penetration = -depth;
    }
    return info;
}
