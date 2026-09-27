#pragma once
#include <glad/glad.h>
#include <string>
#include "math/Vec3.h"
#include "math/Mat4.h"
#include "renderer/Shader.h"

class BitmapFont {
public:
    ~BitmapFont();
    void init();
    void drawText(Shader& fontShader, const std::string& text,
                  float x, float y, float scale, const Vec3& color);
    void setProjection(Shader& fontShader, int screenW, int screenH);

private:
    GLuint m_vao       = 0;
    GLuint m_vbo       = 0;
    GLuint m_textureID = 0;

    static constexpr int kCharW = 8;
    static constexpr int kCharH = 8;
    static constexpr int kCols  = 16;
    static constexpr int kRows  = 8; // chars 0-127

    void buildAtlas();
};
