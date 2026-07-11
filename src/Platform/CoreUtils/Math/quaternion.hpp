// Quaternion type for rotation representation; converted to a Matx4f via QuatRotation() defined in maths.hpp.
#pragma once
#include "vec3.hpp"

class Quaternion {
public:
	float w, x, y, z;

	Quaternion() {
		w = 1.0f;
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

	Quaternion(const Vec3& axis, float angle) {
		float halfAngle = angle * 0.5f;
		float s = sin(halfAngle);
		w = cos(halfAngle);
		x = axis.x * s;
		y = axis.y * s;
		z = axis.z * s;
	}

	Vec3 getVector() const {
		return Vec3(x, y, z);
	}

	Quaternion conjugate() const {
		return Quaternion(w, -x, -y, -z);
	}

	Quaternion operator*(const Quaternion& q) const {
		return Quaternion(
			w * q.w - x * q.x - y * q.y - z * q.z,
			w * q.x + x * q.w + y * q.z - z * q.y,
			w * q.y - x * q.z + y * q.w + z * q.x,
			w * q.z + x * q.y - y * q.x + z * q.w
		);
	}

	Quaternion operator*(const Vec3& v) const {
		return Quaternion(
			-w * v.x - x * v.y - y * v.z,
			w * v.x + x * v.y + y * v.z,
			w * v.y - x * v.x + y * v.z,
			w * v.z + x * v.x - y * v.y
		);
	}

	float length() const {
		return sqrt(w * w + x * x + y * y + z * z);
	}

	static Quaternion identity() {
		return Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
	}
};