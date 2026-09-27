#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Application {
public:
    Application(int width, int height, const char* title);
    ~Application();
    void run();
private:
    GLFWwindow* m_window = nullptr;
    int m_width, m_height;
};
