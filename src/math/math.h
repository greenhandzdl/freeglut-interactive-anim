/**
 * @file math/math.h
 * @brief 数学工具：Vec3、Mat4、PI 常量
 */
#pragma once

#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#define MATH_PI 3.14159265358979323846f

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

// ============================================================
// 4x4 矩阵工具
// ============================================================
struct Mat4 {
    float m[16];

    Mat4() { setIdentity(); }

    void setIdentity() {
        memset(m, 0, 16 * sizeof(float));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    static Mat4 perspective(float fovY, float aspect, float nearZ, float farZ) {
        Mat4 r;
        float f = 1.0f / tanf(fovY * 0.5f * MATH_PI / 180.0f);
        float nf = 1.0f / (nearZ - farZ);
        memset(r.m, 0, 16 * sizeof(float));
        r.m[0]  = f / aspect;
        r.m[5]  = f;
        r.m[10] = (farZ + nearZ) * nf;
        r.m[11] = -1.0f;
        r.m[14] = 2.0f * farZ * nearZ * nf;
        return r;
    }

    static Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up) {
        Vec3 f = { center.x - eye.x, center.y - eye.y, center.z - eye.z };
        float fl = sqrtf(f.x*f.x + f.y*f.y + f.z*f.z);
        if (fl > 0) { f.x /= fl; f.y /= fl; f.z /= fl; }
        Vec3 s = { f.y*up.z - f.z*up.y, f.z*up.x - f.x*up.z, f.x*up.y - f.y*up.x };
        float sl = sqrtf(s.x*s.x + s.y*s.y + s.z*s.z);
        if (sl > 0) { s.x /= sl; s.y /= sl; s.z /= sl; }
        Vec3 u = { s.y*f.z - s.z*f.y, s.z*f.x - s.x*f.z, s.x*f.y - s.y*f.x };

        Mat4 r;
        r.m[0] = s.x;  r.m[4] = s.y;  r.m[8]  = s.z;  r.m[12] = -(s.x*eye.x + s.y*eye.y + s.z*eye.z);
        r.m[1] = u.x;  r.m[5] = u.y;  r.m[9]  = u.z;  r.m[13] = -(u.x*eye.x + u.y*eye.y + u.z*eye.z);
        r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z; r.m[14] =  f.x*eye.x + f.y*eye.y + f.z*eye.z;
        r.m[3] = 0;    r.m[7] = 0;    r.m[11] = 0;    r.m[15] = 1;
        return r;
    }

    static Mat4 translate(float tx, float ty, float tz) {
        Mat4 r;
        r.m[12] = tx; r.m[13] = ty; r.m[14] = tz;
        return r;
    }

    static Mat4 rotateY(float angle) {
        Mat4 r;
        float c = cosf(angle), s = sinf(angle);
        r.m[0] = c;  r.m[2] = s;
        r.m[8] = -s; r.m[10] = c;
        return r;
    }

    Mat4 operator*(const Mat4& b) const {
        Mat4 r;
        for (int j = 0; j < 4; ++j)
            for (int i = 0; i < 4; ++i) {
                float sum = 0;
                for (int k = 0; k < 4; ++k)
                    sum += m[k*4 + i] * b.m[j*4 + k];
                r.m[j*4 + i] = sum;
            }
        return r;
    }
};