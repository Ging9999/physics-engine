#pragma once
#include <glad/glad.h>
#include "renderer/Shader.h"
#include "math/Vec3.h"

class CrosshairRenderer {
public:
    ~CrosshairRenderer();
    void init();
    void draw(Shader& crosshairShader, const Vec3& color);
private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
};
