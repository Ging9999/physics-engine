#pragma once
#include "math/Vec3.h"

struct Ray {
    Vec3 origin;
    Vec3 direction; // normalized
};

class Raycaster {
public:
    static Ray   fromCamera(const Vec3& position, const Vec3& front);

    static float intersectSphere(const Ray& ray, const Vec3& center, float radius);
    static float intersectAABB(const Ray& ray, const Vec3& center, const Vec3& halfExtents);
};
