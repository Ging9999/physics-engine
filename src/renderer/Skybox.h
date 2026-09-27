#pragma once
#include <glad/glad.h>
#include "math/Mat4.h"
#include "renderer/Shader.h"

class Skybox {
public:
    ~Skybox();
    void initProcedural();
    void draw(const Mat4& view, const Mat4& projection, Shader& skyboxShader);

private:
    GLuint m_vao            = 0;
    GLuint m_vbo            = 0;
    GLuint m_cubemapTexture = 0;

    void buildGeometry();
    void buildProceduralCubemap();
};
