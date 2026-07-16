#pragma once
#include "vec3.hpp"
#include <cmath>

class Vec4 {
public:
    float x, y, z, w;
    Vec4() : x(0), y(0), z(0), w(0) { }
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }
    Vec4(Vec2 const& vec) : x(vec.x), y(vec.y), z(0), w(0) { }
    Vec4(Vec3 const& vec) : x(vec.x), y(vec.y), z(vec.z), w(0) { }
    Vec4 operator+(Vec4 const& other) const {
        return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }
    Vec4 operator-(Vec4 const& other) const {
        return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
    }
    Vec4 operator*(float scalar) const {
        return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
    }
    Vec4 operator/(float scalar) const {
        return Vec4(x / scalar, y / scalar, z / scalar, w / scalar);
    }

    Vec4 operator*(Vec4 const& other) const {
        return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    Vec4 operator+=(Vec4 const& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    Vec4 operator-=(Vec4 const& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    float length() const { return sqrt(x * x + y * y + z * z + w * w); }

    Vec4 normalize() const {
        float len = length();
        if (len == 0) return Vec4(0, 0, 0, 0);
        return Vec4(x / len, y / len, z / len, w / len);
    }

    Vec4 dotProduct(Vec4 const& other) const {
        return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
    }
};
