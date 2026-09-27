#include "physics/CollisionResponse.h"
#include <cmath>
#include <algorithm>

static constexpr float kBiasFactor     = 0.2f;
static constexpr float kSlop           = 0.01f;
static constexpr float kFriction       = 0.3f;
static constexpr float kGroundFriction = 0.4f;

void CollisionResponse::resolve(RigidBody& a, RigidBody& b, const CollisionInfo& info) {
    if (!info.collided) return;

    float invA     = a.inverseMass();
    float invB     = b.inverseMass();
    float totalInv = invA + invB;
    if (totalInv <= 0.0f) return;

    a.sleeping = false; a.sleepTimer = 0.0f;
    b.sleeping = false; b.sleepTimer = 0.0f;

    // Baumgarte positional correction
    float corrMag = std::max(info.penetration - kSlop, 0.0f) * kBiasFactor / totalInv;
    Vec3  corr    = info.normal * corrMag;
    a.position   -= corr * invA;
    b.position   += corr * invB;

    // Relative velocity at contact
    Vec3  relVel         = b.velocity - a.velocity;
    float velAlongNormal = relVel.dot(info.normal);
    if (velAlongNormal > 0.0f) return;

    float e = std::min(a.restitution, b.restitution);
    float j = -(1.0f + e) * velAlongNormal / totalInv;
    Vec3  normalImpulse = info.normal * j;

    a.velocity -= normalImpulse * invA;
    b.velocity += normalImpulse * invB;

    // Coulomb friction
    Vec3  tangent    = relVel - info.normal * relVel.dot(info.normal);
    float tangentLen = tangent.length();
    Vec3  frictionImpulse;
    if (tangentLen > 1e-6f) {
        tangent = tangent * (1.0f / tangentLen);
        float jt = -relVel.dot(tangent) / totalInv;
        float maxF = j * kFriction;
        jt = std::max(-maxF, std::min(jt, maxF));
        frictionImpulse = tangent * jt;
        a.velocity -= frictionImpulse * invA;
        b.velocity += frictionImpulse * invB;
    }

    // Torque from contact offset
    Vec3 contactPoint = (a.position + b.position) * 0.5f;
    Vec3 offsetA = contactPoint - a.position;
    Vec3 offsetB = contactPoint - b.position;
    Vec3 totalImpulse = normalImpulse + frictionImpulse;
    a.angularVelocity -= offsetA.cross(totalImpulse) * (invA * 0.5f);
    b.angularVelocity += offsetB.cross(totalImpulse) * (invB * 0.5f);
}

void CollisionResponse::resolveWithGround(RigidBody& body, const CollisionInfo& info) {
    if (!info.collided || body.isStatic) return;

    body.sleeping   = false;
    body.sleepTimer = 0.0f;

    body.position += info.normal * info.penetration;

    float velAlongNormal = body.velocity.dot(info.normal);
    if (velAlongNormal >= 0.0f) return;

    float j = -(1.0f + body.restitution) * velAlongNormal;
    body.velocity += info.normal * j;

    if (std::abs(body.velocity.y) < 0.1f)
        body.velocity.y = 0.0f;

    // Tangential friction
    Vec3  tangent    = body.velocity - info.normal * body.velocity.dot(info.normal);
    float tangentLen = tangent.length();
    if (tangentLen > 1e-6f) {
        float frictionDelta = std::min(tangentLen, j * kGroundFriction);
        body.velocity -= tangent * (frictionDelta / tangentLen);

        // Spin from sliding on ground
        body.angularVelocity += Vec3(-tangent.z, 0.0f, tangent.x) * 0.3f;
    }
}
