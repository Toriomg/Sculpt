#pragma once
#include <GL/glew.h>
#include "vec4.hpp"
#include <cmath>
#include <numbers>

inline constexpr float PI_F = std::numbers::pi_v<float>;

static inline float radians(float degrees) {
    return degrees * PI_F / 180.0f;
}

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

    Vec3 transformPoint(Vec3 point) const {
		Vec4 result;
		result = (*this) * Vec4(point.x, point.y, point.z, 1.0f);
		if (result.w == 0.0f) return Vec3(0.0f, 0.0f, 0.0f);

        float invW = 1.0f / result.w;
        return Vec3(result.x * invW, result.y * invW, result.z * invW);
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

    Vec4 operator*(Vec4 vec) const{
        return Vec4(
            m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z + m[0][3] * vec.w,
            m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z + m[1][3] * vec.w,
            m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z + m[2][3] * vec.w,
            m[3][0] * vec.x + m[3][1] * vec.y + m[3][2] * vec.z + m[3][3] * vec.w
        );
	}

    static Matx4f identity() {
        return Matx4f(1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matx4f translation(const Vec3& vec) {
        return Matx4f(1.0f, 0.0f, 0.0f, vec.x,
                      0.0f, 1.0f, 0.0f, vec.y,
                      0.0f, 0.0f, 1.0f, vec.z,
			          0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matx4f rotationX(float angle) {
        float rad = radians(angle);
        return Matx4f(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, cos(rad), -sin(rad), 0.0f,
                      0.0f, sin(rad),  cos(rad), 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matx4f rotationY(float angle) {
        float rad = radians(angle);
        return Matx4f( cos(rad), 0.0f, sin(rad), 0.0f,
                       0.0f,     1.0f, 0.0f,     0.0f,
                      -sin(rad), 0.0f, cos(rad), 0.0f,
                       0.0f,     0.0f, 0.0f,     1.0f);
    }

    static Matx4f rotationZ(float angle) {
        float rad = radians(angle);
        return Matx4f(cos(rad), -sin(rad), 0.0f, 0.0f,
                      sin(rad),  cos(rad), 0.0f, 0.0f,
                      0.0f,      0.0f,     1.0f, 0.0f,
                      0.0f,      0.0f,     0.0f, 1.0f);
    }

    static Matx4f rotation(const Vec3& axis, float angle) {
        float c = cos(angle);
        float s = sin(angle);
        float t = 1.0f - c;
        return Matx4f(t * axis.x * axis.x + c, t * axis.x * axis.y - s * axis.z, t * axis.x * axis.z + s * axis.y, 0.0f,
                      t * axis.x * axis.y + s * axis.z, t * axis.y * axis.y + c, t * axis.y * axis.z - s * axis.x, 0.0f,
                      t * axis.x * axis.z - s * axis.y, t * axis.y * axis.z + s * axis.x, t * axis.z * axis.z + c, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}

    static Matx4f rotation(float angleX, float angleY, float angleZ) {
        return rotationZ(angleZ) * rotationY(angleY) * rotationX(angleX);
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

    static Matx4f lookAt(const Vec3& position, const Vec3& target, const Vec3& worldUp) {
        // 1. Calculate the forward (Z) axis
        Vec3 forward = (target - position).normalize();

        // 2. Calculate the right (X) axis
        Vec3 right = worldUp.crossProduct(forward).normalize();

        // 3. Calculate the new up (Y) axis
        Vec3 up = forward.crossProduct(right);

        // 4. Create the final view matrix
        return Matx4f(
            right.x, right.y, right.z, -right.dotProduct(position),
            up.x, up.y, up.z, -up.dotProduct(position),
            -forward.x, -forward.y, -forward.z, forward.dotProduct(position), // Negate forward for right-handed systems
            0.0f, 0.0f, 0.0f, 1.0f
        );
	}

    static Matx4f orthographic(float left, float right, float bottom, float top, float zNear, float zFar) {
        return Matx4f(2.0f / (right - left), 0.0f, 0.0f, -(right + left) / (right - left),
                      0.0f, 2.0f / (top - bottom), 0.0f, -(top + bottom) / (top - bottom),
                      0.0f, 0.0f, -2.0f / (zFar - zNear), -(zFar + zNear) / (zFar - zNear),
                      0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matx4f perspective(float fov, float aspectRatio, float zNear, float zFar) {
        float tanHalfFov = tan(fov * PI_F / 360.0f);
        float f = 1.0f / tanHalfFov;

        float A = -(zFar + zNear) / (zFar - zNear);
        float B = -(2.0f * zFar * zNear) / (zFar - zNear);
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

    const float* data() const{
        return &m[0][0];
	}
};