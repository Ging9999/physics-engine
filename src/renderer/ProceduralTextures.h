#pragma once
#include <glad/glad.h>
#include "math/Vec3.h"

class ProceduralTextures {
public:
    static unsigned int createCheckerboard(int size = 256, int squares = 8);
    static unsigned int createBrick(int size = 256);
    static unsigned int createWood(int size = 256);
    static unsigned int createMetal(int size = 256);

private:
    static unsigned int upload(const unsigned char* data, int w, int h, bool repeat = true);
    static unsigned int hash(int x, int y);
};
