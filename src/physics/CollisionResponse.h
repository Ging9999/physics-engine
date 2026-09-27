#pragma once
#include "physics/RigidBody.h"
#include "physics/CollisionDetection.h"

class CollisionResponse {
public:
    static void resolve(RigidBody& a, RigidBody& b, const CollisionInfo& info);
    static void resolveWithGround(RigidBody& body, const CollisionInfo& info);
};
