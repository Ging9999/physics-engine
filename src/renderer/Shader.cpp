#include "renderer/Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::~Shader() {
    if (m_programID) glDeleteProgram(m_programID);
}

bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertSrc = readFile(vertexPath);
    std::string fragSrc = readFile(fragmentPath);

    if (vertSrc.empty() || fragSrc.empty()) {
        std::cerr << "Failed to read shader files\n";
        return false;
    }

    unsigned int vert = compileShader(vertSrc, GL_VERTEX_SHADER);
    unsigned int frag = compileShader(fragSrc, GL_FRAGMENT_SHADER);

    if (!vert || !frag) return false;

    // Link program
    m_programID = glCreateProgram();
    glAttachShader(m_programID, vert);
    glAttachShader(m_programID, frag);
    glLinkProgram(m_programID);
    checkErrors(m_programID, "PROGRAM");

    // Shaders are linked, no longer needed individually
    glDeleteShader(vert);
    glDeleteShader(frag);

    std::cout << "Shader loaded: " << vertexPath << " + " << fragmentPath << "\n";
    return true;
}

void Shader::use() const {
    glUseProgram(m_programID);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(m_programID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(m_programID, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const Vec3& v) const {
    glUniform3f(glGetUniformLocation(m_programID, name.c_str()), v.x, v.y, v.z);
}

void Shader::setMat4(const std::string& name, const Mat4& mat) const {
    glUniformMatrix4fv(
        glGetUniformLocation(m_programID, name.c_str()),
        1, GL_FALSE, mat.data()
    );
}

std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open shader file: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned int Shader::compileShader(const std::string& source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    std::string label = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    checkErrors(shader, label);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void Shader::checkErrors(unsigned int object, const std::string& type) {
    int success;
    char log[1024];

    if (type == "PROGRAM") {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, nullptr, log);
            std::cerr << "Shader LINK error:\n" << log << "\n";
        }
    } else {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, nullptr, log);
            std::cerr << "Shader COMPILE error (" << type << "):\n" << log << "\n";
        }
    }
}
