#include "renderer/DebugRenderer.h"

DebugRenderer::~DebugRenderer() {
    if (m_lineVAO) glDeleteVertexArrays(1, &m_lineVAO);
    if (m_lineVBO) glDeleteBuffers(1, &m_lineVBO);
}

void DebugRenderer::init() {
    glGenVertexArrays(1, &m_lineVAO);
    glGenBuffers(1, &m_lineVBO);

    glBindVertexArray(m_lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(Vec3), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void DebugRenderer::updateLineBuffer(Vec3 start, Vec3 end) {
    Vec3 data[2] = {start, end};
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(Vec3), data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugRenderer::setRay(Vec3 start, Vec3 end, bool hit, Vec3 hitPoint) {
    m_lineStart = start;
    m_lineEnd   = end;
    m_lineColor = hit ? Vec3{0, 1, 0} : Vec3{1, 0, 0};
    m_hitPoint  = hitPoint;
    m_hasHit    = hit;
    m_hasRay    = true;
    updateLineBuffer(start, end);
}

void DebugRenderer::draw(Shader& flatShader, const Mat4& view, const Mat4& projection, Mesh& sphereMesh) {
    if (!visible || !m_hasRay) return;

    flatShader.use();
    flatShader.setMat4("view",       view);
    flatShader.setMat4("projection", projection);
    flatShader.setMat4("model",      Mat4::identity());
    flatShader.setVec3("objectColor", m_lineColor);

    glLineWidth(2.0f);
    glBindVertexArray(m_lineVAO);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
    glLineWidth(1.0f);

    if (m_hasHit) {
        Mat4 model = Mat4::translate(m_hitPoint) * Mat4::scale(Vec3(0.1f, 0.1f, 0.1f));
        flatShader.setMat4("model",      model);
        flatShader.setVec3("objectColor", Vec3{0, 1, 0});
        sphereMesh.draw();
    }
}
