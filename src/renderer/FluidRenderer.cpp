#include "renderer/FluidRenderer.h"
#include "math/Mat4.h"
#include <glad/glad.h>
#include <cmath>
#include <algorithm>

void FluidRenderer::draw(const FluidSystem& fluid, Shader& shader,
                         Mesh& sphereMesh, const Mat4& view, const Mat4& projection) {
    const auto& particles = fluid.getParticles();
    if (particles.empty()) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    shader.use();
    shader.setMat4("view",       view);
    shader.setMat4("projection", projection);
    shader.setFloat("alpha",     0.6f);

    for (const auto& p : particles) {
        if (!p.active) continue;

        Mat4 model = Mat4::translate(p.position) * Mat4::scale({0.15f, 0.15f, 0.15f});
        shader.setMat4("model", model);

        // Colour: slow = deep blue, fast = cyan/white
        float speed = p.velocity.length();
        float t     = std::min(speed / 10.0f, 1.0f);
        Vec3 color  = {0.1f + t * 0.6f, 0.3f + t * 0.5f, 0.9f - t * 0.2f};
        shader.setVec3("objectColor", color);

        sphereMesh.draw();
    }

    glDepthMask(GL_TRUE);
}
