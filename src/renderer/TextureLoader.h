#pragma once
#include <glad/glad.h>
#include <string>

class TextureLoader {
public:
    static unsigned int load(const std::string& filepath, bool repeat = true);
};
