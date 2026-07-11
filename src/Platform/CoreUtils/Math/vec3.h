#pragma once
#include <cmath>
#include "vec2.h"

class Vec3 {
public:
	float x, y, z;
	Vec3() : x(0), y(0), z(0) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	Vec3(const Vec2& vec) : x(vec.x), y(vec.y), z(0) {}
	Vec3 operator+(const Vec3& other) const {
		return Vec3(x + other.x, y + other.y, z + other.z);
	}
	Vec3 operator-(const Vec3& other) const {
		return Vec3(x - other.x, y - other.y, z - other.z);
	}
	Vec3 operator*(float scalar) const {
		return Vec3(x * scalar, y * scalar, z * scalar);
	}
	Vec3 operator/(float scalar) const {
		return Vec3(x / scalar, y / scalar, z / scalar);
	}

	Vec3 operator*(const Vec3& other) const {
		return Vec3(x * other.x, y * other.y, z * other.z);
	}

	Vec3& operator+=(const Vec3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vec3& operator-=(const Vec3& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	Vec3 operator-() const {
		return Vec3(-x, -y, -z);
	}

	float length() const {
		return sqrt(x * x + y * y + z * z);
	}

	Vec3 normalize() const {
		float len = length();
		if (len < 1e-6f) return Vec3(0, 0, 0);
		return Vec3(x / len, y / len, z / len);
	}

	float dotProduct(const Vec3& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	Vec3 crossProduct(const Vec3& other) const {
		/*In Spanish: producto vectorial mixto*/
		return Vec3(
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		);
	}
};