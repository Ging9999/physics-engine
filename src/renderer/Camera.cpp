#include "renderer/Camera.h"
#include <cmath>

static const float PI = 3.14159265359f;
static float radians(float degrees) { return degrees * PI / 180.0f; }

Camera::Camera(Vec3 position, float yaw, float pitch)
    : m_position(position), m_yaw(yaw), m_pitch(pitch) {
    updateVectors();
}

void Camera::processKeyboard(GLFWwindow* window, float deltaTime) {
    float velocity = m_moveSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_position += m_front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_position += m_front * (-velocity);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_position += m_right * (-velocity);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_position += m_right * velocity;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        m_position += m_worldUp * velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        m_position += m_worldUp * (-velocity);
}

void Camera::processMouse(float xOffset, float yOffset) {
    m_yaw   += xOffset * m_mouseSensitivity;
    m_pitch += yOffset * m_mouseSensitivity;

    // Clamp pitch so you don't flip upside down
    if (m_pitch > 89.0f)  m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;

    updateVectors();
}

Mat4 Camera::getViewMatrix() const {
    Vec3 target = m_position + m_front;
    return Mat4::lookAt(m_position, target, m_up);
}

Mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return Mat4::perspective(radians(m_fov), aspectRatio, 0.1f, 100.0f);
}

void Camera::updateVectors() {
    // Calculate new front vector from yaw and pitch
    float yawRad = radians(m_yaw);
    float pitchRad = radians(m_pitch);

    Vec3 front;
    front.x = cos(yawRad) * cos(pitchRad);
    front.y = sin(pitchRad);
    front.z = sin(yawRad) * cos(pitchRad);
    m_front = front.normalized();

    // Recalculate right and up
    m_right = m_front.cross(m_worldUp).normalized();
    m_up = m_right.cross(m_front).normalized();
}
