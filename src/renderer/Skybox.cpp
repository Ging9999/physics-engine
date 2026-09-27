#include "renderer/Skybox.h"
#include <vector>
#include <cmath>

static const float kSkyboxVerts[] = {
    -1, 1,-1,  -1,-1,-1,   1,-1,-1,   1,-1,-1,   1, 1,-1,  -1, 1,-1,
    -1,-1, 1,  -1,-1,-1,  -1, 1,-1,  -1, 1,-1,  -1, 1, 1,  -1,-1, 1,
     1,-1,-1,   1,-1, 1,   1, 1, 1,   1, 1, 1,   1, 1,-1,   1,-1,-1,
    -1,-1, 1,  -1, 1, 1,   1, 1, 1,   1, 1, 1,   1,-1, 1,  -1,-1, 1,
    -1, 1,-1,   1, 1,-1,   1, 1, 1,   1, 1, 1,  -1, 1, 1,  -1, 1,-1,
    -1,-1,-1,  -1,-1, 1,   1,-1,-1,   1,-1,-1,  -1,-1, 1,   1,-1, 1,
};

Skybox::~Skybox() {
    if (m_vao)            glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)            glDeleteBuffers(1, &m_vbo);
    if (m_cubemapTexture) glDeleteTextures(1, &m_cubemapTexture);
}

void Skybox::buildGeometry() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kSkyboxVerts), kSkyboxVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

static std::vector<unsigned char> makeFace(int size, float rTop, float gTop, float bTop,
                                            float rBot, float gBot, float bBot) {
    std::vector<unsigned char> data(size * size * 3);
    for (int y = 0; y < size; y++) {
        float t = (float)y / (size - 1);
        unsigned char r = (unsigned char)(rBot + t * (rTop - rBot));
        unsigned char g = (unsigned char)(gBot + t * (gTop - gBot));
        unsigned char b = (unsigned char)(bBot + t * (bTop - bBot));
        for (int x = 0; x < size; x++) {
            int idx = (y * size + x) * 3;
            data[idx+0] = r; data[idx+1] = g; data[idx+2] = b;
        }
    }
    return data;
}

void Skybox::buildProceduralCubemap() {
    int sz = 64;
    glGenTextures(1, &m_cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapTexture);

    // Horizon haze blue top, deep blue bottom for sides
    float rT=135,gT=206,bT=235;  // sky blue top
    float rH=180,gH=220,bH=240;  // lighter near horizon
    float rB=60, gB=80, bB=120;  // deep blue near bottom

    // side faces: gradient bottom→top
    for (int i = 0; i < 4; i++) {
        auto face = makeFace(sz, rT,gT,bT, rB,gB,bB);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, sz, sz, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, face.data());
    }
    // top face: uniform light sky blue
    auto top = makeFace(sz, rT,gT,bT, rT,gT,bT);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, sz, sz, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, top.data());
    // bottom face: dark ground
    auto bot = makeFace(sz, 65,55,45, 50,45,35);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, sz, sz, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, bot.data());

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Skybox::initProcedural() {
    buildGeometry();
    buildProceduralCubemap();
}

void Skybox::draw(const Mat4& view, const Mat4& projection, Shader& skyboxShader) {
    glDepthFunc(GL_LEQUAL);
    skyboxShader.use();
    skyboxShader.setMat4("projection", projection);
    skyboxShader.setMat4("view",       view);
    skyboxShader.setInt ("skybox",     0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapTexture);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}
