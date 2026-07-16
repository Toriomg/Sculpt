#pragma once
#include "vec2.hpp"
#include <cmath>

class Vec3 {
public:
    float x, y, z;
    Vec3() : x(0), y(0), z(0) { }
    Vec3(float x, float y, float z) : x(x), y(y), z(z) { }
    Vec3(Vec2 const& vec) : x(vec.x), y(vec.y), z(0) { }
    Vec3 operator+(Vec3 const& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator-(Vec3 const& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
    Vec3 operator/(float scalar) const { return Vec3(x / scalar, y / scalar, z / scalar); }

    Vec3 operator*(Vec3 const& other) const { return Vec3(x * other.x, y * other.y, z * other.z); }

    Vec3& operator+=(Vec3 const& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3& operator-=(Vec3 const& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vec3 operator-() const { return Vec3(-x, -y, -z); }

    float length() const { return sqrt(x * x + y * y + z * z); }

    Vec3 normalize() const {
        float len = length();
        if (len < 1e-6f) return Vec3(0, 0, 0);
        return Vec3(x / len, y / len, z / len);
    }

    float dotProduct(Vec3 const& other) const { return x * other.x + y * other.y + z * other.z; }

    Vec3 crossProduct(Vec3 const& other) const {
        /*In Spanish: producto vectorial mixto*/
        return Vec3(y * other.z - z * other.y, z * other.x - x * other.z,
                    x * other.y - y * other.x);
    }
};
