#pragma once
#include "../vec/vec3.h"
#include <GL/glew.h>
#include <cmath>

class Matx4f {
public:
    float m[4][4];

    Matx4f() {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m[i][j] = 0.0f;
            }
        }
    }

    Matx4f(float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33) {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }

    static Matx4f identity() {
        return Matx4f(1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    Matx4f operator*(const Matx4f& other) const {
        Matx4f result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = m[i][0] * other.m[0][j] +
                    m[i][1] * other.m[1][j] +
                    m[i][2] * other.m[2][j] +
                    m[i][3] * other.m[3][j];
            }
        }
        return result;
    }

    Matx4f operator*(float scalar) const {
        Matx4f result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = m[i][j] * scalar;
            }
        }
        return result;
    }

    static Matx4f translation(const Vec3& vec) {
        return Matx4f(1.0f, 0.0f, 0.0f, vec.x,
                      0.0f, 1.0f, 0.0f, vec.y,
                      0.0f, 0.0f, 1.0f, vec.z,
			          0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matx4f rotationX(float angle) {
        return Matx4f(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, cos(angle), -sin(angle), 0.0f,
                      0.0f, sin(angle), cos(angle), 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
	}

    static Matx4f rotationY(float angle) {
        return Matx4f(cos(angle), 0.0f, sin(angle), 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      -sin(angle), 0.0f, cos(angle), 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
	}

    static Matx4f rotationZ(float angle){
        return Matx4f(cos(angle),-sin(angle), 0.0f, 0.0f,
                      sin(angle), cos(angle), 0.0f, 0.0f,
                      0.0f      , 0.0f      , 1.0f, 0.0f,
			          0.0f      , 0.0f      , 0.0f, 1.0f);
    }

    static Matx4f scaling(const Vec3& vec) {
        return Matx4f(vec.x, 0.0f, 0.0f, 0.0f,
                      0.0f, vec.y, 0.0f, 0.0f,
                      0.0f, 0.0f, vec.z, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
	}

    static Matx4f scalingScalar(float scalar) {
        return Matx4f(scalar, 0.0f, 0.0f, 0.0f,
                      0.0f, scalar, 0.0f, 0.0f,
                      0.0f, 0.0f, scalar, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
	}

    static Matx4f orthographic(float left, float right, float bottom, float top, float near, float far) {
        return Matx4f(2.0f / (right - left), 0.0f, 0.0f, -(right + left) / (right - left),
                      0.0f, 2.0f / (top - bottom), 0.0f, -(top + bottom) / (top - bottom),
                      0.0f, 0.0f, -2.0f / (far - near), -(far + near) / (far - near),
                      0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matx4f perspective(float fov, float aspectRatio, float near, float far) {
        float tanHalfFov = tan(fov * 3.141592653589793f / 360.0f);
        float f = 1.0f / tanHalfFov;

        float A = -(far + near) / (far - near);
        float B = -(2.0f * far * near) / (far - near);
        if (aspectRatio == 0.0f) {
            aspectRatio = 1.0f; // Prevent division by zero
		}

        return Matx4f(
            f / aspectRatio, 0.0f, 0.0f, 0.0f,
            0.0f, f, 0.0f, 0.0f,
            0.0f, 0.0f, A, B,
            0.0f, 0.0f, -1.0f, 0.0f
        );
    }

    const float* data() const {
        return &m[0][0];
	}
};