#include "math/Mat4.h"
#include <cmath>

Mat4::Mat4() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m[i][j] = (i == j) ? 1.0f : 0.0f;
}

Mat4 Mat4::identity() { return Mat4(); }

Mat4 Mat4::translate(const Vec3& v) {
    Mat4 r;
    r.m[3][0] = v.x;
    r.m[3][1] = v.y;
    r.m[3][2] = v.z;
    return r;
}

Mat4 Mat4::scale(const Vec3& v) {
    Mat4 r;
    r.m[0][0] = v.x;
    r.m[1][1] = v.y;
    r.m[2][2] = v.z;
    return r;
}

Mat4 Mat4::rotateX(float a) {
    Mat4 r;
    float c = cosf(a), s = sinf(a);
    r.m[1][1] =  c;  r.m[2][1] = -s;
    r.m[1][2] =  s;  r.m[2][2] =  c;
    return r;
}

Mat4 Mat4::rotateY(float a) {
    Mat4 r;
    float c = cosf(a), s = sinf(a);
    r.m[0][0] =  c;  r.m[2][0] =  s;
    r.m[0][2] = -s;  r.m[2][2] =  c;
    return r;
}

Mat4 Mat4::rotateZ(float a) {
    Mat4 r;
    float c = cosf(a), s = sinf(a);
    r.m[0][0] =  c;  r.m[1][0] = -s;
    r.m[0][1] =  s;  r.m[1][1] =  c;
    return r;
}

Mat4 Mat4::perspective(float fov, float asp, float n, float f) {
    Mat4 r;
    float t = tanf(fov / 2.0f);
    r.m[0][0] = 1.0f / (asp * t);
    r.m[1][1] = 1.0f / t;
    r.m[2][2] = -(f + n) / (f - n);
    r.m[2][3] = -1.0f;
    r.m[3][2] = -(2.0f * f * n) / (f - n);
    r.m[3][3] = 0.0f;
    return r;
}

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
    Vec3 f = (target - eye).normalized();
    Vec3 r = f.cross(up).normalized();
    Vec3 u = r.cross(f);

    Mat4 res;
    res.m[0][0] =  r.x;  res.m[1][0] =  r.y;  res.m[2][0] =  r.z;
    res.m[0][1] =  u.x;  res.m[1][1] =  u.y;  res.m[2][1] =  u.z;
    res.m[0][2] = -f.x;  res.m[1][2] = -f.y;  res.m[2][2] = -f.z;
    res.m[3][0] = -(r.dot(eye));
    res.m[3][1] = -(u.dot(eye));
    res.m[3][2] =   f.dot(eye);
    return res;
}

Mat4 Mat4::inverse(const Mat4& src) {
    // General 4x4 inverse via cofactor expansion.
    // Column-major: src.m[col][row].  Entry at math row r, col c → src.m[c][r].
    const auto& s = src.m;

    auto det3 = [](float a00, float a01, float a02,
                   float a10, float a11, float a12,
                   float a20, float a21, float a22) -> float {
        return a00*(a11*a22 - a12*a21)
             - a01*(a10*a22 - a12*a20)
             + a02*(a10*a21 - a11*a20);
    };

    // cof[r][c] = cofactor of entry at (row r, col c)
    float cof[4][4];
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            float mn[3][3];
            int mr = 0;
            for (int sr = 0; sr < 4; sr++) {
                if (sr == r) continue;
                int mc = 0;
                for (int sc = 0; sc < 4; sc++) {
                    if (sc == c) continue;
                    mn[mr][mc] = s[sc][sr]; // entry at math row sr, col sc
                    mc++;
                }
                mr++;
            }
            float d = det3(mn[0][0], mn[0][1], mn[0][2],
                           mn[1][0], mn[1][1], mn[1][2],
                           mn[2][0], mn[2][1], mn[2][2]);
            cof[r][c] = ((r + c) % 2 == 0 ? 1.0f : -1.0f) * d;
        }
    }

    // det = Σ_c  s[c][0] * cof[0][c]
    float det = 0.0f;
    for (int c = 0; c < 4; c++)
        det += s[c][0] * cof[0][c];

    if (fabsf(det) < 1e-8f) return identity();

    float inv = 1.0f / det;
    // M^{-1}_{r,c} = cof[c][r] / det
    // Column-major: result.m[c][r] = cof[c][r] * inv
    Mat4 result;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            result.m[c][r] = cof[c][r] * inv;
    return result;
}

Mat4 Mat4::ortho(float l, float r, float b, float t, float n, float f) {
    Mat4 m;
    m.m[0][0] =  2.0f / (r - l);
    m.m[1][1] =  2.0f / (t - b);
    m.m[2][2] = -2.0f / (f - n);
    m.m[3][0] = -(r + l) / (r - l);
    m.m[3][1] = -(t + b) / (t - b);
    m.m[3][2] = -(f + n) / (f - n);
    return m;
}

Mat4 Mat4::operator*(const Mat4& o) const {
    Mat4 r;
    for (int c = 0; c < 4; c++)
        for (int row = 0; row < 4; row++) {
            r.m[c][row] = 0;
            for (int k = 0; k < 4; k++)
                r.m[c][row] += m[k][row] * o.m[c][k];
        }
    return r;
}
