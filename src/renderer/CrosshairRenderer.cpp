#include "renderer/CrosshairRenderer.h"

CrosshairRenderer::~CrosshairRenderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

void CrosshairRenderer::init() {
    float verts[] = {
        -0.02f,  0.00f,
         0.02f,  0.00f,
         0.00f, -0.03f,
         0.00f,  0.03f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void CrosshairRenderer::draw(Shader& crosshairShader, const Vec3& color) {
    crosshairShader.use();
    crosshairShader.setVec3("crosshairColor", color);

    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}
