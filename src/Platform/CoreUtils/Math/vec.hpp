// Generic 2D/3D/4D float vectors: Vec<2>, Vec<3>, Vec<4> aliased as Vec2, Vec3, Vec4.
// Arithmetic lives once in VecOps<D,N> (CRTP); specializations add named fields via union.
#pragma once

// Anonymous structs inside unions are a Clang/GCC extension (not standard C++).
// Required here so .x/.y/.z/.w remain addressable float fields (e.g. &pos.x for ImGui),
// while data[] enables generic loop-based arithmetic in VecOps. Same pattern as GLM.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"

#include <cmath>
#include <cstddef>

// CRTP base: all N-dimensional arithmetic lives here once. D is the derived Vec<N> specialization.
// Every method casts `this` down to D const& to access D::data[N] (the union float array).
// Returning D (not VecOps<D,N>) means callers see the concrete type, preserving method chaining.
template <typename D, std::size_t N> struct VecOps {
    // Each operator creates a zero-initialized D (calling D's default ctor), fills it via data[],
    // then returns it by value. The cast to D const& is required because VecOps has no data member
    // of its own — the actual storage lives in the union of the derived specialization.
    D operator+(D const& o) const {
        D r{};
        auto const& s = static_cast<D const&>(*this);
        for (std::size_t i = 0; i < N; ++i) r.data[i] = s.data[i] + o.data[i];
        return r;
    }
    D operator-(D const& o) const {
        D r{};
        auto const& s = static_cast<D const&>(*this);
        for (std::size_t i = 0; i < N; ++i) r.data[i] = s.data[i] - o.data[i];
        return r;
    }
    // Scalar multiply. Each specialization also defines operator*(Vec const&) for component-wise
    // multiply — but doing so hides this overload via C++ name-hiding rules. The specializations
    // restore it with `using VecOps<Vec<N>,N>::operator*;`.
    D operator*(float scalar) const {
        D r{};
        auto const& s = static_cast<D const&>(*this);
        for (std::size_t i = 0; i < N; ++i) r.data[i] = s.data[i] * scalar;
        return r;
    }
    D operator/(float scalar) const {
        D r{};
        auto const& s = static_cast<D const&>(*this);
        for (std::size_t i = 0; i < N; ++i) r.data[i] = s.data[i] / scalar;
        return r;
    }
    D operator-() const {
        D r{};
        auto const& s = static_cast<D const&>(*this);
        for (std::size_t i = 0; i < N; ++i) r.data[i] = -s.data[i];
        return r;
    }
    bool operator==(D const& o) const {
        auto const& s = static_cast<D const&>(*this);
        for (std::size_t i = 0; i < N; ++i)
            if (s.data[i] != o.data[i]) return false;
        return true;
    }
    float length() const {
        auto const& s = static_cast<D const&>(*this);
        float sum     = 0.0f;
        for (std::size_t i = 0; i < N; ++i) sum += s.data[i] * s.data[i];
        return std::sqrt(sum);
    }
    // normalize() reuses operator/(float) from this same base — safe because by the time it is
    // instantiated, D is a complete type and its operator/ is resolved through VecOps.
    D normalize() const {
        float len = length();
        if (len < 1e-6f) return D{};
        return static_cast<D const&>(*this) / len;
    }
    float dot(D const& o) const {
        auto const& s = static_cast<D const&>(*this);
        float sum     = 0.0f;
        for (std::size_t i = 0; i < N; ++i) sum += s.data[i] * o.data[i];
        return sum;
    }
};

template <std::size_t N> struct Vec;

template <> struct Vec<2> : VecOps<Vec<2>, 2> {
    union {
        float data[2];
        struct {
            float x, y;
        };
    };

    Vec() : data{} { }
    Vec(float x, float y) : data{x, y} { }

    // Defining operator*(Vec const&) below would hide the VecOps::operator*(float) overload via
    // C++ name-hiding: any operator* in the derived class shadows ALL operator* from the base.
    // The `using` re-imports the scalar overload so both coexist without ambiguity.
    using VecOps<Vec<2>, 2>::operator*;
    Vec operator*(Vec const& o) const { return {x * o.x, y * o.y}; }  // component-wise
    float dotProduct(Vec const& o) const { return dot(o); }           // API alias for dot()

    Vec& operator+=(Vec const& o) {
        x += o.x;
        y += o.y;
        return *this;
    }
    Vec& operator-=(Vec const& o) {
        x -= o.x;
        y -= o.y;
        return *this;
    }
    Vec& operator*=(float s) {
        x *= s;
        y *= s;
        return *this;
    }
};

template <> struct Vec<3> : VecOps<Vec<3>, 3> {
    union {
        float data[3];
        struct {
            float x, y, z;
        };
    };

    Vec() : data{} { }
    Vec(float x, float y, float z) : data{x, y, z} { }
    Vec(Vec<2> const& v) : data{v.x, v.y, 0.0f} { }

    // Same name-hiding fix as Vec<2>: restores VecOps::operator*(float) alongside the
    // component-wise operator*(Vec const&) defined in this specialization.
    using VecOps<Vec<3>, 3>::operator*;
    Vec operator*(Vec const& o) const { return {x * o.x, y * o.y, z * o.z}; }  // component-wise
    float dotProduct(Vec const& o) const { return dot(o); }                    // API alias
    Vec crossProduct(Vec const& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }

    Vec& operator+=(Vec const& o) {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
    Vec& operator-=(Vec const& o) {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        return *this;
    }
};

template <> struct Vec<4> : VecOps<Vec<4>, 4> {
    union {
        float data[4];
        struct {
            float x, y, z, w;
        };
    };

    Vec() : data{} { }
    Vec(float x, float y, float z, float w) : data{x, y, z, w} { }
    Vec(Vec<2> const& v) : data{v.x, v.y, 0.0f, 0.0f} { }
    Vec(Vec<3> const& v) : data{v.x, v.y, v.z, 0.0f} { }

    // Same name-hiding fix: restores VecOps::operator*(float) alongside component-wise operator*.
    using VecOps<Vec<4>, 4>::operator*;
    Vec operator*(Vec const& o) const {
        return {x * o.x, y * o.y, z * o.z, w * o.w};
    }  // component-wise
    // pre-existing naming bug: dotProduct on Vec4 returns component-wise Vec4, not a scalar
    Vec dotProduct(Vec const& o) const { return *this * o; }

    Vec& operator+=(Vec const& o) {
        x += o.x;
        y += o.y;
        z += o.z;
        w += o.w;
        return *this;
    }
    Vec& operator-=(Vec const& o) {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        w -= o.w;
        return *this;
    }
};

using Vec2 = Vec<2>;
using Vec3 = Vec<3>;
using Vec4 = Vec<4>;

#pragma clang diagnostic pop
