#include "renderer/ProceduralTextures.h"
#include <vector>
#include <cmath>

unsigned int ProceduralTextures::hash(int x, int y) {
    unsigned int n = x + y * 57;
    n = (n << 13) ^ n;
    return (n * (n * n * 15731u + 789221u) + 1376312589u) & 0x7fffffff;
}

unsigned int ProceduralTextures::upload(const unsigned char* data, int w, int h, bool repeat) {
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    GLenum wrap = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return id;
}

unsigned int ProceduralTextures::createCheckerboard(int size, int squares) {
    std::vector<unsigned char> data(size * size * 3);
    int sqSize = size / squares;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int idx = (y * size + x) * 3;
            bool white = (((x / sqSize) + (y / sqSize)) % 2) == 0;
            unsigned char v = white ? 230 : 30;
            data[idx] = data[idx+1] = data[idx+2] = v;
        }
    }
    return upload(data.data(), size, size);
}

unsigned int ProceduralTextures::createBrick(int size) {
    std::vector<unsigned char> data(size * size * 3);
    int bH = size / 8;
    int bW = size / 4;
    int mortar = std::max(2, size / 128);

    for (int y = 0; y < size; y++) {
        int row = y / bH;
        bool mortarH = (y % bH) < mortar;
        int offset = (row % 2) * (bW / 2);

        for (int x = 0; x < size; x++) {
            int idx = (y * size + x) * 3;
            int bx = (x + offset) % bW;
            bool mortarV = bx < mortar || bx > bW - mortar;

            if (mortarH || mortarV) {
                data[idx+0] = 175; data[idx+1] = 165; data[idx+2] = 155;
            } else {
                unsigned int h = hash(x, y);
                data[idx+0] = (unsigned char)(150 + (h & 0x1F));
                data[idx+1] = (unsigned char)(65  + ((h >> 5) & 0x1F));
                data[idx+2] = (unsigned char)(55  + ((h >> 10) & 0x0F));
            }
        }
    }
    return upload(data.data(), size, size);
}

unsigned int ProceduralTextures::createWood(int size) {
    std::vector<unsigned char> data(size * size * 3);
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int idx = (y * size + x) * 3;
            float ring = sinf((float)y * 0.25f + (float)x * 0.03f +
                              (float)(hash(x, y) & 0xFF) * 0.005f) * 0.5f + 0.5f;
            float grain = (float)(hash(x*3, y*7) & 0xFF) / 255.0f * 0.08f;
            float t = ring + grain;
            data[idx+0] = (unsigned char)(140 + t * 60);
            data[idx+1] = (unsigned char)(80  + t * 40);
            data[idx+2] = (unsigned char)(35  + t * 20);
        }
    }
    return upload(data.data(), size, size);
}

unsigned int ProceduralTextures::createMetal(int size) {
    std::vector<unsigned char> data(size * size * 3);
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int idx = (y * size + x) * 3;
            // Brushed metal: horizontal streaks + slight noise
            float streak = (float)(hash(0, y) & 0xFF) / 255.0f * 0.15f;
            float noise  = (float)(hash(x, y) & 0xFF) / 255.0f * 0.05f;
            float t = 0.7f + streak + noise;
            unsigned char v = (unsigned char)(std::min(1.0f, t) * 200.0f);
            data[idx+0] = v; data[idx+1] = v; data[idx+2] = (unsigned char)(v * 1.05f < 255 ? v * 1.05f : 255);
        }
    }
    return upload(data.data(), size, size);
}
