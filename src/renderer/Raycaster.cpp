#include "renderer/Raycaster.h"
#include <cmath>
#include <algorithm>

Ray Raycaster::fromCamera(const Vec3& position, const Vec3& front) {
    return Ray{position, front};
}

float Raycaster::intersectSphere(const Ray& ray, const Vec3& center, float radius) {
    Vec3  oc = ray.origin - center;
    float a  = ray.direction.dot(ray.direction);
    float b  = 2.0f * oc.dot(ray.direction);
    float c  = oc.dot(oc) - radius * radius;
    float disc = b*b - 4.0f*a*c;

    if (disc < 0.0f) return -1.0f;

    float sqrtDisc = std::sqrt(disc);
    float t0 = (-b - sqrtDisc) / (2.0f * a);
    float t1 = (-b + sqrtDisc) / (2.0f * a);

    if (t0 > 0.0f) return t0;
    if (t1 > 0.0f) return t1;
    return -1.0f;
}

float Raycaster::intersectAABB(const Ray& ray, const Vec3& center, const Vec3& halfExtents) {
    Vec3 boxMin = center - halfExtents;
    Vec3 boxMax = center + halfExtents;

    float tmin = -1e30f, tmax = 1e30f;

    float dirX = ray.direction.x;
    if (std::abs(dirX) < 1e-8f) {
        if (ray.origin.x < boxMin.x || ray.origin.x > boxMax.x) return -1.0f;
    } else {
        float t1 = (boxMin.x - ray.origin.x) / dirX;
        float t2 = (boxMax.x - ray.origin.x) / dirX;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
    }

    float dirY = ray.direction.y;
    if (std::abs(dirY) < 1e-8f) {
        if (ray.origin.y < boxMin.y || ray.origin.y > boxMax.y) return -1.0f;
    } else {
        float t1 = (boxMin.y - ray.origin.y) / dirY;
        float t2 = (boxMax.y - ray.origin.y) / dirY;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
    }

    float dirZ = ray.direction.z;
    if (std::abs(dirZ) < 1e-8f) {
        if (ray.origin.z < boxMin.z || ray.origin.z > boxMax.z) return -1.0f;
    } else {
        float t1 = (boxMin.z - ray.origin.z) / dirZ;
        float t2 = (boxMax.z - ray.origin.z) / dirZ;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
    }

    if (tmin > tmax || tmax < 0.0f) return -1.0f;
    return (tmin >= 0.0f) ? tmin : tmax;
}
