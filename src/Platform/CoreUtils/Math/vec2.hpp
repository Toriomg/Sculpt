#pragma once
#include <cmath>

class Vec2 {
	public:
	float x, y;
	Vec2() : x(0), y(0) {}
	Vec2(float x, float y) : x(x), y(y) {}
	Vec2 operator+(const Vec2& other) const {
		return Vec2(x + other.x, y + other.y);
	}
	Vec2 operator-(const Vec2& other) const {
		return Vec2(x - other.x, y - other.y);
	}
	Vec2 operator*(float scalar) const {
		return Vec2(x * scalar, y * scalar);
	}
	Vec2 operator/(float scalar) const {
		return Vec2(x / scalar, y / scalar);
	}
	Vec2 operator+=(const Vec2& other) {
		x += other.x;
		y += other.y;
		return *this;
	}
	Vec2 operator-=(const Vec2& other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}
	Vec2 operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	}
	Vec2 operator-() const {
		return Vec2(-x, -y);
	}

	float dotProduct(const Vec2& other) const {
		return x * other.x + y * other.y;
	}

	float length() const {
		return sqrt(x * x + y * y);
	}

	Vec2 normalize() const {
		float len = length();
		if (len == 0) return Vec2(0, 0);
		return Vec2(x / len, y / len);
	}
};