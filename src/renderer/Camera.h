#pragma once
#include "math/Vec3.h"
#include "math/Mat4.h"
#include <GLFW/glfw3.h>

class Camera {
public:
    Camera(Vec3 position, float yaw, float pitch);

    void processKeyboard(GLFWwindow* window, float deltaTime);
    void processMouse(float xOffset, float yOffset);

    Mat4 getViewMatrix()                  const;
    Mat4 getProjectionMatrix(float aspect) const;

    Vec3 getPosition() const { return m_position; }
    Vec3 getFront()    const { return m_front; }

private:
    void updateVectors();

    Vec3 m_position;
    Vec3 m_front;
    Vec3 m_up;
    Vec3 m_right;
    Vec3 m_worldUp = {0, 1, 0};

    float m_yaw;
    float m_pitch;

    float m_moveSpeed        = 5.0f;
    float m_mouseSensitivity = 0.1f;
    float m_fov              = 45.0f;
};
