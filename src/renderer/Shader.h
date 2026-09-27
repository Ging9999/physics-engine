#pragma once
#include <glad/glad.h>
#include <string>
#include "math/Vec3.h"
#include "math/Mat4.h"
#include "renderer/Material.h"

class Shader {
public:
    Shader() = default;
    ~Shader();

    bool load(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;

    // Uniform setters
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const Vec3& value) const;
    void setMat4(const std::string& name, const Mat4& value) const;

    void setMaterial(const Material& mat) const {
        setVec3 ("material_ambient",   mat.ambient);
        setVec3 ("material_diffuse",   mat.diffuse);
        setVec3 ("material_specular",  mat.specular);
        setFloat("material_shininess", mat.shininess);
    }

    unsigned int getID() const { return m_programID; }

private:
    unsigned int m_programID = 0;
    std::string readFile(const std::string& path);
    unsigned int compileShader(const std::string& source, GLenum type);
    void checkErrors(unsigned int object, const std::string& type);
};
