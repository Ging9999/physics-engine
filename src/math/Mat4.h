#pragma once
#include "math/Vec3.h"

class Mat4 {
public:
    // Column-major: m[col][row]
    float m[4][4] = {};

    Mat4();

    static Mat4 identity();
    static Mat4 translate(const Vec3& v);
    static Mat4 scale(const Vec3& v);
    static Mat4 rotateX(float radians);
    static Mat4 rotateY(float radians);
    static Mat4 rotateZ(float radians);
    static Mat4 perspective(float fovRadians, float aspect, float near, float far);
    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up);
    static Mat4 inverse(const Mat4& m);
    static Mat4 ortho(float left, float right, float bottom, float top, float near, float far);

    Mat4 operator*(const Mat4& other) const;

    const float* data() const { return &m[0][0]; }
};
