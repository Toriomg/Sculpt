// Generic N×M float matrix. Mat<4> (= Matx4f) provides all 3D transform and projection factories.
#pragma once
#include "Platform/CoreUtils/Math/vec.hpp"
#include <cmath>
#include <cstddef>
#include <numbers>

inline constexpr float PI_F = std::numbers::pi_v<float>;

static inline float radians(float degrees) {
    return degrees * PI_F / 180.0f;
}

// Satisfied only for Mat<4,4>. Used to gate the 3D-transform factory methods so that calling
// e.g. Mat<3,3>::translation() is a compile error rather than a silent linker mystery.
template <std::size_t N, std::size_t M>
concept Mat4 = (N == 4 && M == 4);

template <std::size_t N, std::size_t M = N> struct Mat {
    float m[N][M]{};

    // Dimensionally-checked mat×mat: N×M * M×P → N×P. The inner P template parameter is deduced
    // from the right-hand operand, so mismatched inner dimensions are a compile error.
    template <std::size_t P> Mat<N, P> operator*(Mat<M, P> const& o) const {
        Mat<N, P> r{};
        for (std::size_t i = 0; i < N; ++i)
            for (std::size_t j = 0; j < P; ++j)
                for (std::size_t k = 0; k < M; ++k) r.m[i][j] += m[i][k] * o.m[k][j];
        return r;
    }

    Mat operator*(float scalar) const {
        Mat r{};
        for (std::size_t i = 0; i < N; ++i)
            for (std::size_t j = 0; j < M; ++j) r.m[i][j] = m[i][j] * scalar;
        return r;
    }

    // Mat×Vec: result[i] = Σ_j m[i][j] * v[j]. Vec dimension is enforced by the M parameter —
    // passing a Vec<3> to a Mat<4,4> is a compile error at the template argument level.
    Vec<N> operator*(Vec<M> const& v) const {
        Vec<N> r{};
        for (std::size_t i = 0; i < N; ++i)
            for (std::size_t j = 0; j < M; ++j) r.data[i] += m[i][j] * v.data[j];
        return r;
    }

    bool operator==(Mat const& o) const {
        for (std::size_t i = 0; i < N; ++i)
            for (std::size_t j = 0; j < M; ++j)
                if (m[i][j] != o.m[i][j]) return false;
        return true;
    }
    bool operator!=(Mat const& o) const { return !(*this == o); }

    float const* data_ptr() const { return &m[0][0]; }

    static Mat identity()
        requires(N == M) {
        Mat r{};
        for (std::size_t i = 0; i < N; ++i) r.m[i][i] = 1.0f;
        return r;
    }

    Vec<3> transformPoint(Vec<3> const& p) const
        requires Mat4<N, M> {
        Vec<4> r = (*this) * Vec<4>{p.x, p.y, p.z, 1.0f};
        if (r.w == 0.0f) return Vec<3>{};
        float invW = 1.0f / r.w;
        return Vec<3>{r.x * invW, r.y * invW, r.z * invW};
    }

    static Mat translation(Vec<3> const& v)
        requires Mat4<N, M> {
        Mat r     = identity();
        r.m[0][3] = v.x;
        r.m[1][3] = v.y;
        r.m[2][3] = v.z;
        return r;
    }

    static Mat scaling(Vec<3> const& v)
        requires Mat4<N, M> {
        Mat r{};
        r.m[0][0] = v.x;
        r.m[1][1] = v.y;
        r.m[2][2] = v.z;
        r.m[3][3] = 1.0f;
        return r;
    }

    static Mat scalingScalar(float s)
        requires Mat4<N, M> {
        return scaling({s, s, s});
    }

    // angle in degrees
    static Mat rotationX(float degrees_)
        requires Mat4<N, M> {
        float r = radians(degrees_);
        float c = std::cos(r), s = std::sin(r);
        Mat mat     = identity();
        mat.m[1][1] = c;
        mat.m[1][2] = -s;
        mat.m[2][1] = s;
        mat.m[2][2] = c;
        return mat;
    }

    // angle in degrees
    static Mat rotationY(float degrees_)
        requires Mat4<N, M> {
        float r = radians(degrees_);
        float c = std::cos(r), s = std::sin(r);
        Mat mat     = identity();
        mat.m[0][0] = c;
        mat.m[0][2] = s;
        mat.m[2][0] = -s;
        mat.m[2][2] = c;
        return mat;
    }

    // angle in degrees
    static Mat rotationZ(float degrees_)
        requires Mat4<N, M> {
        float r = radians(degrees_);
        float c = std::cos(r), s = std::sin(r);
        Mat mat     = identity();
        mat.m[0][0] = c;
        mat.m[0][1] = -s;
        mat.m[1][0] = s;
        mat.m[1][1] = c;
        return mat;
    }

    // angle in radians (unlike rotationX/Y/Z which take degrees)
    static Mat rotation(Vec<3> const& axis, float angle)
        requires Mat4<N, M> {
        float c = std::cos(angle), s = std::sin(angle), t = 1.0f - c;
        float x = axis.x, y = axis.y, z = axis.z;
        Mat mat{};
        mat.m[0][0] = t * x * x + c;
        mat.m[0][1] = t * x * y - s * z;
        mat.m[0][2] = t * x * z + s * y;
        mat.m[1][0] = t * x * y + s * z;
        mat.m[1][1] = t * y * y + c;
        mat.m[1][2] = t * y * z - s * x;
        mat.m[2][0] = t * x * z - s * y;
        mat.m[2][1] = t * y * z + s * x;
        mat.m[2][2] = t * z * z + c;
        mat.m[3][3] = 1.0f;
        return mat;
    }

    // angles in degrees
    static Mat rotation(float xDeg, float yDeg, float zDeg)
        requires Mat4<N, M> {
        return rotationZ(zDeg) * rotationY(yDeg) * rotationX(xDeg);
    }

    static Mat lookAt(Vec<3> const& pos, Vec<3> const& target, Vec<3> const& up)
        requires Mat4<N, M> {
        Vec<3> forward = (target - pos).normalize();
        Vec<3> right   = up.crossProduct(forward).normalize();
        Vec<3> newUp   = forward.crossProduct(right);
        Mat mat{};
        mat.m[0][0] = right.x;
        mat.m[0][1] = right.y;
        mat.m[0][2] = right.z;
        mat.m[0][3] = -right.dotProduct(pos);
        mat.m[1][0] = newUp.x;
        mat.m[1][1] = newUp.y;
        mat.m[1][2] = newUp.z;
        mat.m[1][3] = -newUp.dotProduct(pos);
        mat.m[2][0] = -forward.x;
        mat.m[2][1] = -forward.y;
        mat.m[2][2] = -forward.z;
        mat.m[2][3] = forward.dotProduct(pos);
        mat.m[3][3] = 1.0f;
        return mat;
    }

    static Mat orthographic(float left, float right, float bottom, float top, float zNear,
                            float zFar)
        requires Mat4<N, M> {
        Mat mat{};
        mat.m[0][0] = 2.0f / (right - left);
        mat.m[0][3] = -(right + left) / (right - left);
        mat.m[1][1] = 2.0f / (top - bottom);
        mat.m[1][3] = -(top + bottom) / (top - bottom);
        mat.m[2][2] = -2.0f / (zFar - zNear);
        mat.m[2][3] = -(zFar + zNear) / (zFar - zNear);
        mat.m[3][3] = 1.0f;
        return mat;
    }

    static Mat perspective(float fov, float aspect, float zNear, float zFar)
        requires Mat4<N, M> {
        if (aspect == 0.0f) aspect = 1.0f;
        float f = 1.0f / std::tan(fov * PI_F / 360.0f);
        float A = -(zFar + zNear) / (zFar - zNear);
        float B = -(2.0f * zFar * zNear) / (zFar - zNear);
        Mat mat{};
        mat.m[0][0] = f / aspect;
        mat.m[1][1] = f;
        mat.m[2][2] = A;
        mat.m[2][3] = B;
        mat.m[3][2] = -1.0f;
        return mat;
    }
};

using Matx4f = Mat<4>;
