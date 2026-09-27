#pragma once
#include "math/Vec3.h"

struct CollisionInfo {
    bool  collided    = false;
    Vec3  normal;
    float penetration = 0.0f;
};

class CollisionDetection {
public:
    static CollisionInfo sphereVsSphere(const Vec3& posA, float rA, const Vec3& posB, float rB);
    static CollisionInfo aabbVsAABB(const Vec3& posA, const Vec3& halfA,
                                    const Vec3& posB, const Vec3& halfB);
    static CollisionInfo sphereVsAABB(const Vec3& spherePos, float radius,
                                      const Vec3& boxPos, const Vec3& halfExtents);
    static CollisionInfo sphereVsGround(const Vec3& pos, float radius);
    static CollisionInfo aabbVsGround(const Vec3& pos, const Vec3& halfExtents);
};
