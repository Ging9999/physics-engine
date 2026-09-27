#pragma once
#include "math/Vec3.h"

struct FluidParticle {
    Vec3  position;
    Vec3  velocity;
    Vec3  force;
    float density  = 0.0f;
    float pressure = 0.0f;
    bool  active   = true;
};
