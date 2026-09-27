#pragma once
#include "renderer/Mesh.h"
#include "renderer/Shader.h"
#include "physics/FluidSystem.h"
#include "math/Mat4.h"

class FluidRenderer {
public:
    void draw(const FluidSystem& fluid, Shader& shader,
              Mesh& sphereMesh, const Mat4& view, const Mat4& projection);
};
