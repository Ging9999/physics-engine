#include "physics/PhysicsWorld.h"
#include "scene/SceneObject.h"
#include "physics/CollisionDetection.h"
#include "physics/CollisionResponse.h"
#include <cmath>

static constexpr int kSolverIterations = 8;

void PhysicsWorld::step(float dt, std::vector<SceneObject>& objects) {
    if (paused) return;
    collisionEvents.clear();
    applyGravity(objects);
    integrateAll(dt, objects);
    for (int iter = 0; iter < kSolverIterations; iter++)
        detectAndResolveCollisions(objects);
    clearAllForces(objects);
}

void PhysicsWorld::applyGravity(std::vector<SceneObject>& objects) {
    for (int i = 0; i < (int)objects.size(); i++) {
        auto& obj = objects[i];
        if (!obj.rigidBody.isStatic) {
            float scale = (i == grabbedObjectIndex) ? 0.2f : 1.0f;
            obj.rigidBody.applyForce(gravity * (obj.rigidBody.mass * scale));
        }
    }
}

void PhysicsWorld::integrateAll(float dt, std::vector<SceneObject>& objects) {
    for (auto& obj : objects)
        obj.rigidBody.integrate(dt);
}

void PhysicsWorld::detectAndResolveCollisions(std::vector<SceneObject>& objects) {
    for (auto& obj : objects) {
        CollisionInfo info;
        if (obj.collider.type == ColliderType::SPHERE)
            info = CollisionDetection::sphereVsGround(obj.rigidBody.position, obj.collider.sphere.radius);
        else
            info = CollisionDetection::aabbVsGround(obj.rigidBody.position, obj.collider.aabb.halfExtents);
        if (info.collided)
            CollisionResponse::resolveWithGround(obj.rigidBody, info);
    }

    for (size_t i = 0; i < objects.size(); i++) {
        for (size_t j = i + 1; j < objects.size(); j++) {
            auto& a  = objects[i];
            auto& b  = objects[j];
            auto  tA = a.collider.type;
            auto  tB = b.collider.type;

            CollisionInfo info;
            if (tA == ColliderType::SPHERE && tB == ColliderType::SPHERE) {
                info = CollisionDetection::sphereVsSphere(
                    a.rigidBody.position, a.collider.sphere.radius,
                    b.rigidBody.position, b.collider.sphere.radius);
            } else if (tA == ColliderType::AABB && tB == ColliderType::AABB) {
                info = CollisionDetection::aabbVsAABB(
                    a.rigidBody.position, a.collider.aabb.halfExtents,
                    b.rigidBody.position, b.collider.aabb.halfExtents);
            } else if (tA == ColliderType::SPHERE && tB == ColliderType::AABB) {
                info = CollisionDetection::sphereVsAABB(
                    a.rigidBody.position, a.collider.sphere.radius,
                    b.rigidBody.position, b.collider.aabb.halfExtents);
            } else {
                info = CollisionDetection::sphereVsAABB(
                    b.rigidBody.position, b.collider.sphere.radius,
                    a.rigidBody.position, a.collider.aabb.halfExtents);
                info.normal = -info.normal;
            }

            if (info.collided) {
                Vec3  relVel  = b.rigidBody.velocity - a.rigidBody.velocity;
                float speed   = std::abs(relVel.dot(info.normal));

                CollisionResponse::resolve(a.rigidBody, b.rigidBody, info);

                if (speed > 1.0f) {
                    collisionEvents.push_back({
                        (a.rigidBody.position + b.rigidBody.position) * 0.5f,
                        a.color, b.color, speed
                    });
                }
            }
        }
    }
}

void PhysicsWorld::clearAllForces(std::vector<SceneObject>& objects) {
    for (auto& obj : objects)
        obj.rigidBody.clearForces();
}
